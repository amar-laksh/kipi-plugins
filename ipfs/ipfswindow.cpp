/* ============================================================
 *
 * This file is a part of KDE project
 *
 *
 * Date        : 2016-06-06
 * Description : a kipi plugin to export images to the IPFS web service
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 * Copyright (C) 2016 by Fabian Vogt <fabian at ritter dash vogt dot de>
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

#include "ipfswindow.h"

// Qt includes

#include <QDesktopServices>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QBoxLayout>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>
#include <kwindowconfig.h>
#include <kconfig.h>

// Local includes

#include "kipiplugins_debug.h"
#include "kpimageinfo.h"
#include "kpaboutdata.h"
#include "kpversion.h"

static const constexpr char *IMGUR_CLIENT_ID("bd2572bce74b73d"),
                            *IMGUR_CLIENT_SECRET("300988683e99cb7b203a5889cf71de9ac891c1c1");

namespace KIPIIPFSPlugin
{

IPFSWindow::IPFSWindow(QWidget* const /*parent*/)
    : KPToolDialog(0)
{
    api = new IPFSGLOBALUPLOADAPI(QString::fromLatin1(IMGUR_CLIENT_ID),
                        QString::fromLatin1(IMGUR_CLIENT_SECRET), this);

    /* Connect API signals */
    connect(api, &IPFSGLOBALUPLOADAPI::authorized, this, &IPFSWindow::apiAuthorized);
    connect(api, &IPFSGLOBALUPLOADAPI::authError,  this, &IPFSWindow::apiAuthError);
    connect(api, &IPFSGLOBALUPLOADAPI::progress,   this, &IPFSWindow::apiProgress);
    connect(api, &IPFSGLOBALUPLOADAPI::requestPin, this, &IPFSWindow::apiRequestPin);
    connect(api, &IPFSGLOBALUPLOADAPI::success,    this, &IPFSWindow::apiSuccess);
    connect(api, &IPFSGLOBALUPLOADAPI::error,      this, &IPFSWindow::apiError);
    connect(api, &IPFSGLOBALUPLOADAPI::busy,       this, &IPFSWindow::apiBusy);

    /* | List | Auth | */
    auto* mainLayout = new QHBoxLayout;
    auto* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    this->setMainWidget(mainWidget);

    this->list = new IPFSImagesList;
    mainLayout->addWidget(list);

    /* |  Logged in as:  |
     * | <Not logged in> |
     * |     Forget      | */

    auto* userLabelLabel = new QLabel(i18n("Custom Logged in as:"));
    userLabelLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    userLabelLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    this->userLabel = new QLabel; /* Label set in readSettings() */
    userLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    userLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    forgetButton = new QPushButton(i18n("Forget"));

    auto* authLayout = new QVBoxLayout;
    mainLayout->addLayout(authLayout);
    authLayout->addWidget(userLabelLabel);
    authLayout->addWidget(userLabel);
    authLayout->addWidget(forgetButton);
    authLayout->insertStretch(-1, 1);

    /* Add anonymous upload button */
    uploadAnonButton = new QPushButton(i18n("Custom Upload Anonymously"));
    addButton(uploadAnonButton, QDialogButtonBox::ApplyRole);

    /* Connect UI signals */
    connect(forgetButton, &QPushButton::clicked,
            this, &IPFSWindow::forgetButtonClicked);
    connect(startButton(), &QPushButton::clicked,
            this, &IPFSWindow::slotUpload);
    connect(uploadAnonButton, &QPushButton::clicked,
            this, &IPFSWindow::slotAnonUpload);
    connect(this, &IPFSWindow::finished,
            this, &IPFSWindow::slotFinished);
    connect(this, &IPFSWindow::cancelClicked,
            this, &IPFSWindow::slotCancel);

    setWindowIcon(QIcon::fromTheme(QString::fromLatin1("kipi-ipfs")));
    setWindowTitle(i18n("Export to IPFS"));
    setModal(false);

    startButton()->setText(i18n("Upload"));
    startButton()->setToolTip(i18n("Start upload to IPFS"));
    startButton()->setEnabled(true);

    /* Add about data */

    KPAboutData* const about = new KPAboutData(ki18n("IPFS Export"),
                                   ki18n("A tool to export images to IPFS web service"),
                                   ki18n("(c) 2012-2013, Marius Orcsik"));

    about->addAuthor(ki18n("Marius Orcsik").toString(),
                     ki18n("Author").toString(),
                     QString::fromLatin1("marius at habarnam dot ro"));

    about->addAuthor(ki18n("Gilles Caulier").toString(),
                     ki18n("Developer").toString(),
                     QString::fromLatin1("caulier dot gilles at gmail dot com"));

    about->addAuthor(ki18n("Fabian Vogt").toString(),
                     ki18n("Developer").toString(),
                     QString::fromLatin1("fabian at ritter dash vogt dot de"));

    about->setHandbookEntry(QString::fromLatin1("tool-ipfsexport"));
    setAboutData(about);

    /* Only used if not overwritten by readSettings() */
    resize(650, 320);
    readSettings();
}

IPFSWindow::~IPFSWindow()
{
    saveSettings();
}

void IPFSWindow::reactivate()
{
    list->loadImagesFromCurrentSelection();
    show();
}

void IPFSWindow::forgetButtonClicked()
{
    api->getAuth().unlink();

    apiAuthorized(false, {});
}

void IPFSWindow::slotUpload()
{
    QList<const IPFSImageListViewItem*> pending = this->list->getPendingItems();

    for (auto item : pending)
    {
        IPFSGLOBALUPLOADAPIAction action;
        action.type = IPFSGLOBALUPLOADAPIActionType::IMG_UPLOAD;
        action.upload.imgpath = item->url().toLocalFile();
        action.upload.title = item->Title();
        action.upload.description = item->Description();

        api->queueWork(action);
    }
}

void IPFSWindow::slotAnonUpload()
{
    QList<const IPFSImageListViewItem*> pending = this->list->getPendingItems();

    for (auto item : pending)
    {
        IPFSGLOBALUPLOADAPIAction action;
        action.type = IPFSGLOBALUPLOADAPIActionType::ANON_IMG_UPLOAD;
        action.upload.imgpath = item->url().toLocalFile();
        action.upload.title = item->Title();
        action.upload.description = item->Description();

        api->queueWork(action);
    }
}

void IPFSWindow::slotFinished()
{
    saveSettings();
}

void IPFSWindow::slotCancel()
{
    api->cancelAllWork();
}

void IPFSWindow::apiAuthorized(bool success, const QString& username)
{
    if (success)
    {
        this->username = username;
        this->userLabel->setText(this->username);
        this->forgetButton->setEnabled(true);
        return;
    }

    this->username = QString();
    this->userLabel->setText(i18n("<Not logged in>"));
    this->forgetButton->setEnabled(false);
}

void IPFSWindow::apiAuthError(const QString& msg)
{
    QMessageBox::critical(this,
                          i18n("Authorization Failed"),
                          i18n("Failed to log into IPFS: %1\n", msg));
}

void IPFSWindow::apiProgress(unsigned int /*percent*/, const IPFSGLOBALUPLOADAPIAction& action)
{
    list->processing(QUrl::fromLocalFile(action.upload.imgpath));
}

void IPFSWindow::apiRequestPin(const QUrl& url)
{
    QDesktopServices::openUrl(url);
}

void IPFSWindow::apiSuccess(const IPFSGLOBALUPLOADAPIResult& result)
{
    list->slotSuccess(result);
}

void IPFSWindow::apiError(const QString& msg, const IPFSGLOBALUPLOADAPIAction& action)
{
    list->processed(QUrl::fromLocalFile(action.upload.imgpath), false);

    /* 1 here because the current item is still in the queue. */
    if (api->workQueueLength() <= 1)
    {
        QMessageBox::critical(this,
                              i18n("Uploading Failed"),
                              i18n("Failed to upload photo to IPFS: %1\n", msg));
        return;
    }

    QMessageBox::StandardButton cont =
            QMessageBox::question(this,
                                  i18n("Uploading Failed"),
                                  i18n("Failed to upload photo to IPFS: %1\n"
                                       "Do you want to continue?", msg));

    if (cont != QMessageBox::Yes)
        api->cancelAllWork();
}

void IPFSWindow::apiBusy(bool busy)
{
    setCursor(busy ? Qt::WaitCursor : Qt::ArrowCursor);
    startButton()->setEnabled(!busy);
}

void IPFSWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    slotFinished();
    e->accept();
}

void IPFSWindow::readSettings()
{
    KConfig config(QString::fromLatin1("kipirc"));
    KConfigGroup groupAuth = config.group("IPFS Auth");
    username = groupAuth.readEntry("username", QString());
    apiAuthorized(!username.isEmpty(), username);

    winId();
    KConfigGroup groupDialog = config.group("IPFS Dialog");
    KWindowConfig::restoreWindowSize(windowHandle(), groupDialog);
    resize(windowHandle()->size());
}

void IPFSWindow::saveSettings()
{
    KConfig config(QString::fromLatin1("kipirc"));
    KConfigGroup groupAuth = config.group("IPFS Auth");
    groupAuth.writeEntry("username", username);

    KConfigGroup groupDialog = config.group("IPFS Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), groupDialog);
    config.sync();
}

} // namespace KIPIIPFSPlugin
