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

static const QString ipfs_auth_url = QLatin1String("https://api.ipfs.com/oauth2/authorize");

static const QString ipfs_upload_url = QLatin1String("https://api.globalupload.io/transport/add"),

ipfs_token_url = QLatin1String("https://api.ipfs.com/oauth2/token");
static const uint16_t ipfs_redirect_port = 8000; // Redirect URI is http://127.0.0.1:8000

IPFSGLOBALUPLOADAPI::IPFSGLOBALUPLOADAPI(const QString& client_id, const QString& client_secret, QObject* parent)
    : QObject(parent)
{
    m_auth.setClientId(client_id);
    m_auth.setClientSecret(client_secret);
    m_auth.setRequestUrl(ipfs_auth_url);
    m_auth.setTokenUrl(ipfs_token_url);
    m_auth.setRefreshTokenUrl(ipfs_token_url);
    m_auth.setLocalPort(ipfs_redirect_port);
    m_auth.setLocalhostPolicy(QString());

    QString kipioauth = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/kipioauthrc");

    QSettings* const settings    = new QSettings(kipioauth, QSettings::IniFormat, this);
    O0SettingsStore* const store = new O0SettingsStore(settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("IPFS"));
    m_auth.setStore(store);

    connect(&m_auth, &O2::linkedChanged, this, &IPFSGLOBALUPLOADAPI::oauthAuthorized);
    connect(&m_auth, &O2::openBrowser,   this, &IPFSGLOBALUPLOADAPI::oauthRequestPin);
    connect(&m_auth, &O2::linkingFailed, this, &IPFSGLOBALUPLOADAPI::oauthFailed);
}

IPFSGLOBALUPLOADAPI::~IPFSGLOBALUPLOADAPI()
{
    /* Disconnect all signals as cancelAllWork may emit */
    disconnect(this, 0, 0, 0);
    cancelAllWork();
}

O2 &IPFSGLOBALUPLOADAPI::getAuth()
{
    return m_auth;
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

QUrl IPFSGLOBALUPLOADAPI::urlForDeletehash(const QString& deletehash)
{
    return QUrl{QLatin1String("https://ipfs.com/delete/") + deletehash};
}

void IPFSGLOBALUPLOADAPI::oauthAuthorized()
{
    bool success = m_auth.linked();

    if (success)
        startWorkTimer();
    else
        emit busy(false);

    emit authorized(success, m_auth.extraTokens()[QLatin1String("account_username")].toString());
}

void IPFSGLOBALUPLOADAPI::oauthRequestPin(const QUrl& url)
{
    emit busy(false);
    emit requestPin(url);
}

void IPFSGLOBALUPLOADAPI::oauthFailed()
{
    emit authError(i18n("Could not authorize"));
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
        auto data = response.object()[QLatin1String("data")].toObject();

        switch (result.action->type)
        {
            case IPFSGLOBALUPLOADAPIActionType::IMG_UPLOAD:
            case IPFSGLOBALUPLOADAPIActionType::ANON_IMG_UPLOAD:
                result.image.animated = data[QLatin1String("animated")].toBool();
                result.image.bandwidth = data[QLatin1String("bandwidth")].toInt();
                result.image.datetime = data[QLatin1String("datetime")].toInt();
                result.image.deletehash = data[QLatin1String("deletehash")].toString();
                result.image.description = data[QLatin1String("description")].toString();
                result.image.height = data[QLatin1String("height")].toInt();
                result.image.hash = data[QLatin1String("id")].toString();
                result.image.name = data[QLatin1String("name")].toString();
                result.image.size = data[QLatin1String("size")].toInt();
                result.image.title = data[QLatin1String("title")].toString();
                result.image.type = data[QLatin1String("type")].toString();
                result.image.url = data[QLatin1String("link")].toString();
                result.image.views = data[QLatin1String("views")].toInt();
                result.image.width = data[QLatin1String("width")].toInt();
                break;
            case IPFSGLOBALUPLOADAPIActionType::ACCT_INFO:
                result.account.username = data[QLatin1String("url")].toString();
                /* TODO: Other fields */
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

            m_auth.refresh();
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

void IPFSGLOBALUPLOADAPI::addAuthToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Bearer %1").arg(m_auth.token()).toUtf8());
}

void IPFSGLOBALUPLOADAPI::addAnonToken(QNetworkRequest* request)
{
    request->setRawHeader(QByteArray("Authorization"),
                          QString::fromLatin1("Client-ID %1").arg(m_auth.clientId()).toUtf8());
}

void IPFSGLOBALUPLOADAPI::doWork()
{
    if (m_work_queue.empty() || m_reply != nullptr)
        return;

    auto &work = m_work_queue.front();

    if (work.type != IPFSGLOBALUPLOADAPIActionType::ANON_IMG_UPLOAD && !m_auth.linked())
    {
        m_auth.link();
        return; /* Wait for the authorized() signal. */
    }

    switch(work.type)
    {
        case IPFSGLOBALUPLOADAPIActionType::ACCT_INFO:
        {
            QNetworkRequest request(QUrl(QString::fromLatin1("https://api.ipfs.com/3/account/%1")
                                        .arg(QLatin1String(work.account.username.toUtf8().toPercentEncoding()))));
            addAuthToken(&request);

            this->m_reply = m_net.get(request);
            break;
        }
        case IPFSGLOBALUPLOADAPIActionType::ANON_IMG_UPLOAD:
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
            title.setBody(work.upload.title.toUtf8().toPercentEncoding());
            multipart->append(title);

            QHttpPart description;
            description.setHeader(QNetworkRequest::ContentDispositionHeader,
                                  QLatin1String("form-data; name=\"user\""));
            description.setBody(work.upload.description.toUtf8().toPercentEncoding());
            multipart->append(description);

            QHttpPart image;
            image.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QString::fromLatin1("form-data; file=\"%1\"")
                            .arg(QLatin1String(QFileInfo(work.upload.imgpath).fileName().toUtf8().toPercentEncoding()))));
            image.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
            image.setBodyDevice(this->m_image);
            multipart->append(image);

            QNetworkRequest request(QUrl(QLatin1String("https://api.ipfs.com/3/image")));

            if (work.type == IPFSGLOBALUPLOADAPIActionType::IMG_UPLOAD)
                addAuthToken(&request);
            else
                addAnonToken(&request);

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
