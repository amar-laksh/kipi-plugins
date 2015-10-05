/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-09-13
 * Description : a plugin to export images to flash
 *
 * Copyright (C) 2011 by Veaceslav Munteanu <slavuttici at gmail dot com>
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

#include "generalpage.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QStandardPaths>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>
#include <KDCRAW/RNumInput>

//Local includes

#include "simpleviewer.h"

using namespace KDcrawIface;

namespace KIPIFlashExportPlugin
{

class GeneralPage::Private
{
public:

    Private()
    {
        title              = 0;
        resizeExportImages = 0;
        showComments       = 0;
        imagesExportSize   = 0;
        maxImageDimension  = 0;
        exportUrl          = 0;
        rightClick         = 0;
        fixOrientation     = 0;
        openInKonqueror    = 0;
        showKeywords       = 0;
    }

    QLineEdit*     title;

    QCheckBox*     resizeExportImages;
    QCheckBox*     showComments;
    QCheckBox*     rightClick;
    QCheckBox*     fixOrientation;
    QCheckBox*     openInKonqueror;
    QCheckBox*     showKeywords;

    RIntNumInput*  imagesExportSize;
    RIntNumInput*  maxImageDimension;

    RFileSelector* exportUrl;
};

GeneralPage::GeneralPage(KPWizardDialog* const dlg)
    : KPWizardPage(dlg, i18n("General Settings")),
      d(new Private)
{
    RVBox* vbox             = new RVBox(this);

    // ------------------------------------------------------------------------

    QGroupBox* const box    = new QGroupBox(i18n("Gallery &Title"), vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(box);
    d->title                = new QLineEdit(this);
    d->title->setWhatsThis(i18n("Enter here the gallery title"));

    vlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->addWidget(d->title);

    // ------------------------------------------------------------------------

    QGroupBox* const box2    = new QGroupBox(i18n("Save Gallery To"), vbox);
    QVBoxLayout* const vlay2 = new QVBoxLayout(box2);
    d->exportUrl             = new RFileSelector(this);
    d->exportUrl->lineEdit()->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QLatin1String("/simpleviewer"));
    d->exportUrl->setFileDlgMode(QFileDialog::Directory);

    vlay2->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay2->addWidget(d->exportUrl);

    // ------------------------------------------------------------------------

    QGroupBox* box3       = new QGroupBox(i18n("Image Properties"), vbox);
    QGridLayout* grid     = new QGridLayout(box3);
    d->resizeExportImages = new QCheckBox(i18n("Resize Target Images"), this);
    d->resizeExportImages->setChecked(true);
    d->resizeExportImages->setWhatsThis(i18n("If you enable this option, "
                                             "all target images can be resized.") );


    d->fixOrientation = new QCheckBox(i18n("Auto-Rotate/Flip Images"), this);
    d->fixOrientation->setChecked(true);
    d->fixOrientation->setWhatsThis(i18n("If you enable this option, "
                                         "the images' orientations will be set according "
                                         "to their Exif information."));

    RHBox* hbox         = new RHBox;
    QLabel* label       = new QLabel(i18n("&Target Images' Size:"), hbox);
    d->imagesExportSize = new RIntNumInput(hbox);
    d->imagesExportSize->setRange(200, 2000, 1);
    d->imagesExportSize->setValue(640);
    d->imagesExportSize->setWhatsThis(i18n("The new size of the exported images, in pixels. "
                                           "SimpleViewer resizes the images too, but this "
                                           "resizes the images before they are uploaded to your server."));
    label->setBuddy(d->imagesExportSize);

    connect(d->resizeExportImages, SIGNAL(toggled(bool)),
            d->imagesExportSize, SLOT(setEnabled(bool)));

    connect(d->resizeExportImages, SIGNAL(toggled(bool)),
            d->fixOrientation, SLOT(setEnabled(bool)));

    RHBox* hbox2         = new RHBox;
    QLabel* label2       = new QLabel(i18n("&Displayed Images' Size:"), hbox2);
    d->maxImageDimension = new RIntNumInput(hbox2);
    d->maxImageDimension->setRange(200, 2000, 1);
    d->maxImageDimension->setValue(640);
    d->maxImageDimension->setWhatsThis(i18n("Scales the displayed images to this size. Normally, use "
                                            "the height or width of your largest image (in pixels). "
                                            "Images will not be scaled up above this size, to "
                                            "preserve image quality."));
    label2->setBuddy(d->maxImageDimension);

    grid->addWidget(d->resizeExportImages, 0, 0, 1, 2);
    grid->addWidget(d->fixOrientation,     1, 1, 1, 1);
    grid->addWidget(hbox,                  2, 1, 1, 1);
    grid->addWidget(hbox2,                 3, 0, 1, 2);
    grid->setColumnMinimumWidth(0, QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // ------------------------------------------------------------------------

    QGroupBox* box4    = new QGroupBox(i18n("Misc"), vbox);
    QVBoxLayout* vlay4 = new QVBoxLayout(box4);

    d->showComments = new QCheckBox(i18n("Display Captions"), this);
    d->showComments->setChecked(true);
    d->showComments->setWhatsThis(i18n("If this option is enabled, "
                                       "the images' captions will be shown."));
    d->showKeywords = new QCheckBox(i18n("Display Keywords"), this);
    d->showKeywords->setChecked(false);
    d->showKeywords->setWhatsThis(i18n("If this option is enabled, "
                                       "the images' keywords will be shown."));

    d->rightClick = new QCheckBox(i18n("Open Image with Right Click"), this);
    d->rightClick->setChecked(false);
    d->rightClick->setWhatsThis(i18n("If this option is enabled, "
                                     "the user will be able to open the target image in a separate window "
                                     "using the right mouse button."));

    d->openInKonqueror = new QCheckBox(i18n("Open gallery in Browser"), this);
    d->openInKonqueror->setChecked(true);
    d->openInKonqueror->setWhatsThis(i18n("If this option is enabled, the "
                                          "gallery will be opened in Konqueror automatically."));


    vlay4->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay4->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay4->addWidget(d->showComments);
    vlay4->addWidget(d->rightClick);
    vlay4->addWidget(d->openInKonqueror);
    vlay4->addWidget(d->showKeywords);

    // ------------------------------------------------------------------------

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QStringLiteral("kipi-flash")).pixmap(128));
}

GeneralPage::~GeneralPage()
{
    delete d;
}

void GeneralPage::setSettings(const SimpleViewerSettingsContainer* const settings)
{
    d->title->setText(settings->title);
    d->exportUrl->lineEdit()->setText(settings->exportUrl.toLocalFile());
    d->resizeExportImages->setChecked(settings->resizeExportImages);
    d->imagesExportSize->setValue(settings->imagesExportSize);
    d->maxImageDimension->setValue(settings->maxImageDimension);
    d->showComments->setChecked(settings->showComments);
    d->rightClick->setChecked(settings->enableRightClickOpen);
    d->fixOrientation->setChecked(settings->fixOrientation);
    d->openInKonqueror->setChecked(settings->openInKonqueror);
    d->showKeywords->setChecked(settings->showKeywords);
}

void GeneralPage::settings(SimpleViewerSettingsContainer* const settings)
{
    settings->title                = d->title->text();
    settings->exportUrl            = QUrl::fromLocalFile(d->exportUrl->lineEdit()->text());
    settings->resizeExportImages   = d->resizeExportImages->isChecked();
    settings->imagesExportSize     = d->imagesExportSize->value();
    settings->maxImageDimension    = d->maxImageDimension->value();
    settings->showComments         = d->showComments->isChecked();
    settings->enableRightClickOpen = d->rightClick->isChecked();
    settings->fixOrientation       = d->fixOrientation->isChecked();
    settings->openInKonqueror      = d->openInKonqueror->isChecked();
    settings->showKeywords         = d->showKeywords->isChecked();
}

}   // namespace KIPIFlashExportPlugin
