/* ============================================================
 *
 * This file is a part of KDE project
 *
 *
 * Date        : 2012-02-12
 * Description : a kipi plugin to export images to the IPFS web service
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
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

#ifndef IMGURWINDOW_H
#define IMGURWINDOW_H

// Qt includes

#include <QObject>
#include <QLabel>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "ipfsimageslist.h"
#include "kptooldialog.h"
#include "ipfsglobaluploadapi.h"

namespace KIPI
{
    class Interface;
}

using namespace KIPI;
using namespace KIPIPlugins;

namespace KIPIIPFSPlugin
{

class IPFSWindow : public KPToolDialog
{
    Q_OBJECT

public:
    IPFSWindow(QWidget* const parent = 0);
    ~IPFSWindow();

    void reactivate();

public Q_SLOTS:
    /* UI callbacks */
    void forgetButtonClicked();
    void slotUpload();
    void slotAnonUpload();
    void slotFinished();
    void slotCancel();

    /* IPFSGLOBALUPLOADAPI callbacks */
    void apiAuthorized(bool success, const QString& username);
    void apiAuthError(const QString& msg);
    void apiProgress(unsigned int percent, const IPFSGLOBALUPLOADAPIAction& action);
    void apiRequestPin(const QUrl& url);
    void apiSuccess(const IPFSGLOBALUPLOADAPIResult& result);
    void apiError(const QString &msg, const IPFSGLOBALUPLOADAPIAction& action);
    void apiBusy(bool busy);

private:
    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:
    IPFSImagesList* list = nullptr;
    IPFSGLOBALUPLOADAPI*       api  = nullptr;
    QPushButton*     forgetButton = nullptr;
    QPushButton*     uploadAnonButton = nullptr;
    QLabel*          userLabel = nullptr;
    /* Contains the ipfs username if API authorized.
     * If not, username is null. */
    QString          username;
};

} // namespace KIPIIPFSPlugin

#endif /* IMGURWINDOW_H */
