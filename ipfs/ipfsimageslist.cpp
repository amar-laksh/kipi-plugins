
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

#include "ipfsimageslist.h"

// C++ includes

#include <memory>

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "kipiplugins_debug.h"

namespace KIPIIPFSPlugin
{

IPFSImagesList::IPFSImagesList(QWidget* const parent)
    : KPImagesList(parent)
{
    setControlButtonsPlacement(KPImagesList::ControlButtonsBelow);
    setAllowDuplicate(false);
    setAllowRAW(false);

    auto* list = listView();

    list->setColumnLabel(KPImagesListView::Thumbnail, i18n("Thumbnail"));

    list->setColumnLabel(static_cast<KIPIPlugins::KPImagesListView::ColumnType>(IPFSImagesList::Title),
                         i18n("Submission title"));

    list->setColumnLabel(static_cast<KIPIPlugins::KPImagesListView::ColumnType>(IPFSImagesList::Description),
                         i18n("Submission description"));

    list->setColumn(static_cast<KIPIPlugins::KPImagesListView::ColumnType>(IPFSImagesList::URL),
                    i18n("IPFS URL"), true);

    connect(list, &KPImagesListView::itemDoubleClicked,
            this, &IPFSImagesList::slotDoubleClick);
}

QList<const IPFSImageListViewItem*> IPFSImagesList::getPendingItems()
{
    QList<const IPFSImageListViewItem*> ret;

    for (unsigned int i = listView()->topLevelItemCount(); i--;)
    {
        const auto* item = dynamic_cast<const IPFSImageListViewItem*>(listView()->topLevelItem(i));

        if (item && item->IPFSUrl().isEmpty())
            ret << item;
    }

    return ret;
}

void IPFSImagesList::slotAddImages(const QList<QUrl>& list)
{
    /* Replaces the KPImagesList::slotAddImages method, so that
     * IPFSImageListViewItems can be added instead of ImagesListViewItems */

    std::unique_ptr<MetadataProcessor> meta;

    if (iface())
        meta = std::unique_ptr<MetadataProcessor>(iface()->createMetadataProcessor());

    for (QList<QUrl>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        // Already in the list?
        if (listView()->findItem(*it) == nullptr)
        {
            auto* item = new IPFSImageListViewItem(listView(), *it);

            // Load URLs from meta data, if possible
            if (meta && meta->load(*it))
            {
                item->setIPFSUrl(meta->getXmpTagString(QLatin1String("Xmp.kipi.IPFSId")));
            }
        }
    }

    emit signalImageListChanged();
    emit signalAddItems(list);
}

void IPFSImagesList::slotSuccess(const IPFSGLOBALUPLOADAPIResult& result)
{
    QUrl ipfsl = QUrl::fromLocalFile(result.action->upload.imgpath);

    processed(ipfsl, true);

    Interface* intf = iface();

    if (intf)
    {
        QPointer<MetadataProcessor> meta = intf->createMetadataProcessor();

        // Save URLs to meta data, if possible
        if (meta && meta->load(ipfsl))
        {
            meta->setXmpTagString(QLatin1String("Xmp.kipi.IPFSId"),         result.image.url);
            bool saved = meta->applyChanges();
            qCDebug(KIPIPLUGINS_LOG) << "Metadata" << (saved ? "Saved" : "Not Saved") << "to" << ipfsl;
        }
    }

    IPFSImageListViewItem* const currItem = dynamic_cast<IPFSImageListViewItem*>(listView()->findItem(ipfsl));

    if (!currItem)
        return;

    if (!result.image.url.isEmpty())
        currItem->setIPFSUrl(result.image.url);
}

void IPFSImagesList::slotDoubleClick(QTreeWidgetItem* element, int i)
{
    if (i == URL )
    {
        const QUrl url = QUrl(element->text(i));
        // The delete page asks for confirmation, so we don't need to do that here
        QDesktopServices::openUrl(url);
    }
}

// ------------------------------------------------------------------------------------------------

IPFSImageListViewItem::IPFSImageListViewItem(KPImagesListView* const view, const QUrl& url)
    : KPImagesListViewItem(view, url)
{
    const QColor blue(50, 50, 255);

    setTextColor(IPFSImagesList::URL, blue);
}

void IPFSImageListViewItem::setTitle(const QString& str)
{
    setText(IPFSImagesList::Title, str);
}

QString IPFSImageListViewItem::Title() const
{
    return text(IPFSImagesList::Title);
}

void IPFSImageListViewItem::setDescription(const QString& str)
{
    setText(IPFSImagesList::Description, str);
}

QString IPFSImageListViewItem::Description() const
{
    return text(IPFSImagesList::Description);
}

void IPFSImageListViewItem::setIPFSUrl(const QString& str)
{
    setText(IPFSImagesList::URL, str);
}

QString IPFSImageListViewItem::IPFSUrl() const
{
    return text(IPFSImagesList::URL);
}

} // namespace KIPIIPFSPlugin
