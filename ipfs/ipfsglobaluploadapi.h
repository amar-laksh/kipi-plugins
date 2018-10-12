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

#ifndef IMGURGLOBALUPLOADAPI_H
#define IMGURGLOBALUPLOADAPI_H

// C++ includes

#include <atomic>
#include <queue>

// Qt includes

#include <QNetworkAccessManager>
#include <QString>
#include <QFile>
#include <QUrl>

// Local includes

#include "o2.h"

enum class IPFSGLOBALUPLOADAPIActionType
{
    ACCT_INFO, /* Action: account Result: account */
    IMG_UPLOAD, /* Action: upload Result: image */
    ANON_IMG_UPLOAD, /* Action: upload Result: image */
};

struct IPFSGLOBALUPLOADAPIAction
{
    IPFSGLOBALUPLOADAPIActionType type;
    struct
    {
        QString imgpath;
        QString title;
        QString description;
    } upload;

    struct
    {
        QString username;
    } account;
};

struct IPFSGLOBALUPLOADAPIResult
{
    const IPFSGLOBALUPLOADAPIAction* action;

    struct IPFSImage
    {
        QString    name;
        QString    title;
        QString    hash;
        QString    deletehash;
        QString    url;
        QString    description;
        qulonglong datetime;
        QString    type;
        bool       animated;
        uint       width;
        uint       height;
        uint       size;
        uint       views;
        qulonglong bandwidth;
    } image;

    struct IPFSAccount
    {
        QString username;
    } account;
};

/* Main class, handles the client side of the IPFS API v3. */
class IPFSGLOBALUPLOADAPI : public QObject
{
Q_OBJECT

public:
    IPFSGLOBALUPLOADAPI(const QString& client_id, const QString& client_secret, QObject* parent = nullptr);
    ~IPFSGLOBALUPLOADAPI();

    /* Use this to read/write the access and refresh tokens. */
    O2 &getAuth();

    unsigned int workQueueLength();
    void queueWork(const IPFSGLOBALUPLOADAPIAction& action);
    void cancelAllWork();

    static QUrl urlForDeletehash(const QString& deletehash);

Q_SIGNALS:
    /* Called if authentication state changes. */
    void authorized(bool success, const QString& username);
    void authError(const QString& msg);

    /* Open url in a browser and let the user copy the pin.
     * Call setPin(pin) to authorize. */
    void requestPin(const QUrl& url);

    /* Emitted on progress changes. */
    void progress(unsigned int percent, const IPFSGLOBALUPLOADAPIAction& action);
    void success(const IPFSGLOBALUPLOADAPIResult& result);
    void error(const QString& msg, const IPFSGLOBALUPLOADAPIAction& action);

    /* Emitted when the status changes. */
    void busy(bool b);

public Q_SLOTS:
    /* Connected to m_auth.linkedChanged(). */
    void oauthAuthorized();
    /* Connected to m_auth.openBrowser(QUrl). */
    void oauthRequestPin(const QUrl& url);
    /* Connected to m_auth.linkingFailed(). */
    void oauthFailed();

    /* Connected to the current QNetworkReply. */
    void uploadProgress(qint64 sent, qint64 total);
    void replyFinished();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    /* Starts m_work_timer if m_work_queue not empty. */
    void startWorkTimer();
    /* Stops m_work_timer if running. */
    void stopWorkTimer();
    /* Adds the user authorization info to the request. */
    void addAuthToken(QNetworkRequest* request);
    /* Adds the client authorization info to the request. */
    void addAnonToken(QNetworkRequest* request);

    /* Start working on the first item of m_work_queue
     * by sending a request. */
    void doWork();

    /* Handler for OAuth 2 related requests. */
    O2 m_auth;

    /* Work queue. */
    std::queue<IPFSGLOBALUPLOADAPIAction> m_work_queue;
    /* ID of timer triggering on idle (0ms). */
    int m_work_timer = 0;

    /* Current QNetworkReply */
    QNetworkReply* m_reply = nullptr;

    /* Current image being uploaded */
    QFile* m_image = nullptr;

    /* The QNetworkAccessManager used for connections */
    QNetworkAccessManager m_net;
};

#endif // IMGURGLOBALUPLOADAPI_H
