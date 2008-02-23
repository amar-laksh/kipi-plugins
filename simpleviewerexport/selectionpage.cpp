/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-02-21
 * Description : collections selection settings page.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <QLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// KIPI include files

#include <libkipi/imagecollectionselector.h>

// Local includes.

#include "selectionpage.h"
#include "selectionpage.moc"

namespace KIPISimpleViewerExportPlugin
{

class SelectionPagePriv
{
public:

    SelectionPagePriv()
    {
        imageCollectionSelector = 0;
    }
    
    KIPI::ImageCollectionSelector *imageCollectionSelector;
};

SelectionPage::SelectionPage(KIPI::Interface* interface, QWidget* parent)
             : QWidget(parent)
{
    d = new SelectionPagePriv;

    QVBoxLayout *layout        = new QVBoxLayout(this);
    d->imageCollectionSelector = new KIPI::ImageCollectionSelector(this, interface);

    layout->addWidget(d->imageCollectionSelector);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->setAlignment(Qt::AlignTop);
}

SelectionPage::~SelectionPage()
{
    delete d;
}

QList<KIPI::ImageCollection> SelectionPage::selection() const
{
    return d->imageCollectionSelector->selectedImageCollections();
}
                                    
}  // namespace KIPISimpleViewerExportPlugin
