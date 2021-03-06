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

#include "plugin_ipfs.h"

// Qt includes

#include <QApplication>
#include <QAction>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kwindowsystem.h>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "kipiplugins_debug.h"

using namespace KIPIPlugins;

namespace KIPIIPFSPlugin
{

K_PLUGIN_FACTORY( IPFSFactory, registerPlugin<Plugin_IPFS>(); )

class Plugin_IPFS::Private
{
public:

    Private()
    {
        actionExport = 0;
        winExport    = 0;
    }

    QAction*     actionExport;
    IPFSWindow* winExport;
};

Plugin_IPFS::Plugin_IPFS(QObject* const parent, const QVariantList& /*args*/)
    : Plugin(parent, "IPFS"),
      d(new Private)
{
    qCDebug(KIPIPLUGINS_LOG) << "IPFS plugin loaded";

    setUiBaseName("kipiplugin_ipfsui.rc");
    setupXML();
}

Plugin_IPFS::~Plugin_IPFS()
{
    delete d->winExport;
    delete d;
}

void Plugin_IPFS::setup(QWidget* const widget)
{
    d->winExport = 0;

    Plugin::setup(widget);

    if (!interface())
    {
        qCCritical(KIPIPLUGINS_LOG) << "Kipi interface is null!";
        return;
    }

    setupActions();
}

void Plugin_IPFS::setupActions()
{
    setDefaultCategory(ExportPlugin);

    d->actionExport = new QAction(this);
    d->actionExport->setText(i18n("Export to &IPFS..."));
    d->actionExport->setIcon(QIcon::fromTheme(QString::fromLatin1("kipi-ipfs")));

    connect(d->actionExport, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));

    addAction(QString::fromLatin1("ipfsexport"), d->actionExport);
}

void Plugin_IPFS::slotActivate()
{
    if (!d->winExport)
    {
        // We clean it up in the close button
        d->winExport = new IPFSWindow(QApplication::activeWindow());
    }
    else
    {
        if (d->winExport->isMinimized())
        {
            KWindowSystem::unminimizeWindow(d->winExport->winId());
        }

        KWindowSystem::activateWindow(d->winExport->winId());
    }

    d->winExport->reactivate();

    qCDebug(KIPIPLUGINS_LOG) << "We have activated the ipfs exporter!";
}

} // namespace KIPIIPFSPlugin

#include "plugin_ipfs.moc"
