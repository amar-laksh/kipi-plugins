/* ============================================================
 *
 * This file is a part of KDE project
 *
 *
 * Date        : 2016-06-06
 * Description : a kipi plugin to export images to the IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya  at xaviers dot edu dot in>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "ipfsglobaluploadapi.h"

// Qt includes

#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QUrlQuery>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "kipiplugins_debug.h"
#include "o0settingsstore.h"
#include "o0globals.h"

static const QString ipfs_upload_url = QLatin1String("https://api.globalupload.io/transport/add");

IPFSGLOBALUPLOADAPI::IPFSGLOBALUPLOADAPI(QObject* parent)
    : QObject(parent)
{
}

IPFSGLOBALUPLOADAPI::~IPFSGLOBALUPLOADAPI()
{
    /* Disconnect all signals as cancelAllWork may emit */
    disconnect(this, 0, 0, 0);
    cancelAllWork();
}

unsigned int IPFSGLOBALUPLOADAPI::workQueueLength()
{
    return m_work_queue.size();
}

void IPFSGLOBALUPLOADAPI::queueWork(const IPFSGLOBALUPLOADAPIAction& action)
{
    m_work_queue.push(action);
    startWorkTimer();
}

void IPFSGLOBALUPLOADAPI::cancelAllWork()
{
    stopWorkTimer();

    if (m_reply)
        m_reply->abort();

    /* Should error be emitted for those actions? */
    while (!m_work_queue.empty())
        m_work_queue.pop();
}

void IPFSGLOBALUPLOADAPI::uploadProgress(qint64 sent, qint64 total)
{
    if (total > 0) /* Don't divide by 0 */
        emit progress((sent * 100) / total, m_work_queue.front());
}

void IPFSGLOBALUPLOADAPI::replyFinished()
{
    auto* reply = m_reply;
    reply->deleteLater();
    m_reply = nullptr;

    if (this->m_image)
    {
        delete this->m_image;
        this->m_image = nullptr;
    }

    if (m_work_queue.empty())
    {
        qCDebug(KIPIPLUGINS_LOG) << "Received result without request";
        return;
    }

    /* toInt() returns 0 if conversion fails. That fits nicely already. */
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    auto response = QJsonDocument::fromJson(reply->readAll());
    if (code == 200 && !response.isEmpty())
    {
        /* Success! */
        IPFSGLOBALUPLOADAPIResult result;
        result.action = &m_work_queue.front();
        switch (result.action->type)
        {
            case IPFSGLOBALUPLOADAPIActionType::IMG_UPLOAD:
                result.image.name = response.object()[QLatin1String("Name")].toString();
                result.image.size = response.object()[QLatin1String("Size")].toInt();
                result.image.url = QLatin1String("https://ipfs.io/ipfs/") + response.object()[QLatin1String("Hash")].toString();
                break;
            default:
                qCWarning(KIPIPLUGINS_LOG) << "Unexpected action";
                qCDebug(KIPIPLUGINS_LOG) << response.toJson();
                break;
        }

        emit success(result);
    }
    else
    {
        if (code == 403)
        {
            /* HTTP 403 Forbidden -> Invalid token? 
             * That needs to be handled internally, so don't emit progress
             * and keep the action in the queue for later retries. */
            return;
        }
        else
        {
            /* Failed. */
            auto msg = response.object()[QLatin1String("data")]
                       .toObject()[QLatin1String("error")]
                       .toString(QLatin1String("Could not read response."));

            emit error(msg, m_work_queue.front());
        }
    }

    /* Next work item. */
    m_work_queue.pop();
    startWorkTimer();
}

void IPFSGLOBALUPLOADAPI::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != m_work_timer)
        return QObject::timerEvent(event);

    event->accept();

    /* One-shot only. */
    QObject::killTimer(event->timerId());
    m_work_timer = 0;

    doWork();
}

void IPFSGLOBALUPLOADAPI::startWorkTimer()
{
    if (!m_work_queue.empty() && m_work_timer == 0)
    {
        m_work_timer = QObject::startTimer(0);
        emit busy(true);
    }
    else
        emit busy(false);
}

void IPFSGLOBALUPLOADAPI::stopWorkTimer()
{
    if (m_work_timer != 0)
    {
        QObject::killTimer(m_work_timer);
        m_work_timer = 0;
    }
}

void IPFSGLOBALUPLOADAPI::doWork()
{
    if (m_work_queue.empty() || m_reply != nullptr)
        return;

    auto &work = m_work_queue.front();

    switch(work.type)
    {
        case IPFSGLOBALUPLOADAPIActionType::IMG_UPLOAD:
        {
            this->m_image = new QFile(work.upload.imgpath);

            if (!m_image->open(QIODevice::ReadOnly))
            {
                delete this->m_image;
                this->m_image = nullptr;

                /* Failed. */
                emit error(i18n("Could not open file"), m_work_queue.front());

                m_work_queue.pop();
                return doWork();
            }

            /* Set ownership to m_image to delete that as well. */
            auto* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, m_image);
            QHttpPart title;
            title.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QLatin1String("form-data; name=\"keyphrase\""));
            multipart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"user\""));
            multipart->append(description);

            QHttpPart image;
            image.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; name=\"file\";  filename=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            image.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("image/jpeg"));
            image.setBodyDevice(this->m_image);
            multipart->append(image);
            QNetworkRequest request(QUrl(QLatin1String("https://api.globalupload.io/transport/add")));
            this->m_reply = this->m_net.post(request, multipart);

            break;
        }
    }

    if (this->m_reply)
    {
        connect(m_reply, &QNetworkReply::uploadProgress, this, &IPFSGLOBALUPLOADAPI::uploadProgress);
        connect(m_reply, &QNetworkReply::finished, this, &IPFSGLOBALUPLOADAPI::replyFinished);
    }
}
