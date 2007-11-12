/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-10-18
 * Description : images list settings page.
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QPushButton>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kdebug.h>

// LibKIPI includes.

#include <libkipi/imagecollection.h>
#include <libkipi/interface.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>
#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "imagespage.h"
#include "imagespage.moc"

namespace KIPISendimagesPlugin
{

ImagesListViewItem::ImagesListViewItem(K3ListView *view, KIPI::Interface *iface, const KUrl& url)
                  : K3ListViewItem(view)
{
    KIPI::ImageInfo imageInfo = iface->info(url);
    
    setThumb(SmallIcon("empty", KIconLoader::SizeLarge, KIconLoader::DisabledState));

    setUrl(url);
    setComments(imageInfo.description());

    setTags(QStringList());
    if (iface->hasFeature(KIPI::HostSupportsTags))
    {
        QMap<QString, QVariant> attribs = imageInfo.attributes();
        setTags(attribs["tags"].toStringList());    
    }

    setRating(-1);
    if (iface->hasFeature(KIPI::HostSupportsRating))
    {
        QMap<QString, QVariant> attribs = imageInfo.attributes();
        setRating(attribs["rating"].toInt());    
    }
}

ImagesListViewItem::~ImagesListViewItem()
{
}

void ImagesListViewItem::setUrl(const KUrl& url)
{
    m_item.orgUrl = url;
    setText(1, m_item.orgUrl.fileName());
}

KUrl ImagesListViewItem::url() 
{
    return m_item.orgUrl; 
}

void ImagesListViewItem::setComments(const QString& comments)
{
    m_item.comments = comments;
    setText(2, m_item.comments);
}

QString ImagesListViewItem::comments() 
{ 
    return m_item.comments; 
}

void ImagesListViewItem::setTags(const QStringList& tags) 
{ 
    m_item.tags = tags; 
}

QStringList ImagesListViewItem::tags() 
{ 
    return m_item.tags; 
}

void ImagesListViewItem::setRating(int rating) 
{ 
    m_item.rating = rating; 
}

int ImagesListViewItem::rating() 
{ 
    return m_item.rating; 
}

void ImagesListViewItem::setThumb(const QPixmap& pix) 
{ 
    setPixmap(0, pix); 
}

EmailItem ImagesListViewItem::emailItem() 
{ 
    return m_item; 
}

// ---------------------------------------------------------------------------

class ImagesPagePriv
{
public:

    ImagesPagePriv()
    {
        listView     = 0;
        iface        = 0;
        addButton    = 0;
        removeButton = 0;
    }

    QPushButton     *addButton;
    QPushButton     *removeButton;

    K3ListView      *listView;

    KIPI::Interface *iface;
};

ImagesPage::ImagesPage(QWidget* parent, KIPI::Interface *iface)
          : QWidget(parent)
{
    d = new ImagesPagePriv;
    d->iface = iface;

    QGridLayout* grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->listView = new K3ListView(this);
    d->listView->addColumn( i18n("Thumb") );
    d->listView->addColumn( i18n("Name") );
    d->listView->addColumn( i18n("Comments") );
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setSelectionMode(Q3ListView::Multi);
    d->listView->setFullWidth(true);
    d->listView->setWhatsThis(i18n("<p>This is the list of images to e-mail."));

    // --------------------------------------------------------

    d->addButton    = new QPushButton(this);
    d->removeButton = new QPushButton(this);
    d->addButton->setText( i18n( "&Add" ) );
    d->addButton->setIcon(SmallIcon("edit-add"));
    d->removeButton->setText( i18n( "&Remove" ) );
    d->removeButton->setIcon(SmallIcon("edit-delete"));

    // --------------------------------------------------------

    grid->addWidget(d->listView, 0, 0, 3, 1);
    grid->addWidget(d->addButton, 0, 1, 1, 1);
    grid->addWidget(d->removeButton, 1, 1, 1, 1);
    grid->setColumnStretch(0, 10);                     
    grid->setRowStretch(2, 10);    
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());        

    // --------------------------------------------------------

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddItems()));

    connect(d->removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveItems()));         

    connect(d->iface, SIGNAL(gotThumbnail( const KUrl&, const QPixmap& )),
            this, SLOT(slotThumbnail(const KUrl&, const QPixmap&)));
}

ImagesPage::~ImagesPage()
{
    delete d;
}

void ImagesPage::addImages(const KUrl::List& list)
{
    if ( list.count() == 0 ) return;

    for( KUrl::List::ConstIterator it = list.begin(); it != list.end(); ++it )
    {
        KUrl imageUrl = *it;
    
        // Check if the new item already exist in the list.
    
        bool find = false;

        Q3ListViewItemIterator it(d->listView);
        for ( ; it.current(); ++it )
        {
            ImagesListViewItem* item = dynamic_cast<ImagesListViewItem*>(it.current());
    
            if (item->url() == imageUrl)
                find = true;
        }
    
        if (!find)
        {
            new ImagesListViewItem(d->listView, d->iface, imageUrl);
            d->iface->thumbnail(imageUrl, 64);
        }
    }
}

void ImagesPage::slotThumbnail(const KUrl& url, const QPixmap& pix)
{
    Q3ListViewItemIterator it(d->listView);
    for ( ; it.current(); ++it )
    {
        ImagesListViewItem* item = dynamic_cast<ImagesListViewItem*>(it.current());
        if (item->url() == url)
        {
            if (pix.isNull())
                item->setThumb(SmallIcon("empty", 64, KIconLoader::DisabledState));
            else
                item->setThumb(pix.scaled(64, 64, Qt::KeepAspectRatio));

            return;
        }
    }
}

void ImagesPage::slotAddItems()
{
    QString fileformats;
   
    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n', QString::SkipEmptyParts);
    
    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];
    
    // Add other files format witch are missing to All Images" type mime provided by KDE and remplace current.
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        allPictures.insert(allPictures.indexOf("|"), QString(raw_file_extentions) + QString(" *.JPE *.TIF"));
        patternList.removeAll(patternList[0]);
        patternList.prepend(allPictures);
    }
    
    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unavailable(see file #121242 in B.K.O).
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
        patternList.append(i18n("\n%1|Camera RAW files",QString(raw_file_extentions)));
    
    fileformats = patternList.join("\n");
    
    KUrl::List urls = KFileDialog::getOpenUrls(d->iface->currentAlbum().path(), 
                                               fileformats, this, i18n("Add Images"));

    if (!urls.isEmpty())
        addImages(urls);
}

void ImagesPage::slotRemoveItems()
{
    bool find;
    do
    {
        find = false;
        Q3ListViewItemIterator it(d->listView);
        for ( ; it.current(); ++it )
        {
            ImagesListViewItem* item = dynamic_cast<ImagesListViewItem*>(it.current());
            if (item->isSelected())
            {
                delete item;
                find = true;
                break;
            }
        }
    }
    while(find);
}

QList<EmailItem> ImagesPage::imagesList()
{
    QList<EmailItem> list;
    Q3ListViewItemIterator it(d->listView);
    for ( ; it.current(); ++it )
    {
        ImagesListViewItem* item = dynamic_cast<ImagesListViewItem*>(it.current());
        list.append(item->emailItem());
    }
    return list;
}

}  // namespace KIPISendimagesPlugin
