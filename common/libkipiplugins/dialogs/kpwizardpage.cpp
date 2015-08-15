/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizzard page.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kpwizardpage.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kassistantdialog.h>
#include <kpagewidgetmodel.h>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

using namespace KDcrawIface;

namespace KIPIPlugins
{

class KPWizardPage::Private
{
public:

    Private()
    {
        hlay          = 0;
        page          = 0;
        logo          = 0;
        leftBottomPix = 0;
    }

    QLabel*          logo;
    QLabel*          leftBottomPix;

    QHBoxLayout*     hlay;

    KPageWidgetItem* page;
};

KPWizardPage::KPWizardPage(KAssistantDialog* const dlg, const QString& title)
    : QScrollArea(dlg),
      d(new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    d->hlay                    = new QHBoxLayout(panel);
    QWidget* const vbox        = new QWidget(panel);
    QVBoxLayout* const vboxLay = new QVBoxLayout(vbox);
    d->logo                    = new QLabel(vbox);
    d->logo->setAlignment(Qt::AlignTop);
    d->logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                      QString("kf5/kipi/pics/kipi-logo.svg")))
                               .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const space = new QLabel(vbox);
    d->leftBottomPix    = new QLabel(vbox);
    d->leftBottomPix->setAlignment(Qt::AlignBottom);

    vboxLay->addWidget(d->logo);
    vboxLay->addWidget(space);
    vboxLay->addWidget(d->leftBottomPix);
    vboxLay->setStretchFactor(space, 10);
    vboxLay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vboxLay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    RLineWidget* const line = new RLineWidget(Qt::Vertical, panel);

    d->hlay->addWidget(vbox);
    d->hlay->addWidget(line);
    d->hlay->setMargin(0);
    d->hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    d->page = dlg->addPage(this, title);
}

KPWizardPage::~KPWizardPage()
{
    delete d;
}

KPageWidgetItem* KPWizardPage::page() const
{
    return d->page;
}

void KPWizardPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}
void KPWizardPage::removePageWidget(QWidget* const w)
{
    d->hlay->removeWidget(w);
}

void KPWizardPage::setLeftBottomPix(const QPixmap& pix)
{
    d->leftBottomPix->setPixmap(pix);
}

}   // namespace KIPIPlugins
