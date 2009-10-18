/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2004-10-01
 * Description : a kipi plugin to batch process images
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RESIZEIMAGESDIALOG_H
#define RESIZEIMAGESDIALOG_H

// Qt includes

#include "qmap.h"
#include "qpair.h"

// Local includes

#include "batchprocessimagesdialog.h"
#include "kpaboutdata.h"
#include "resizeoptionsdialog.h"
#include "resizecommandbuilder.h"

namespace KIPIBatchProcessImagesPlugin
{

class BatchProcessImagesItem;

class ResizeImagesDialog : public BatchProcessImagesDialog
{
    Q_OBJECT

public:

    ResizeImagesDialog(KUrl::List images, KIPI::Interface* interface, QWidget *parent = 0);
    ~ResizeImagesDialog();

private Q_SLOTS:

    void slotHelp(void);
    void slotOptionsClicked(void);

protected:

    void initProcess(KProcess* proc, BatchProcessImagesItem *item,
                     const QString& albumDest, bool previewMode);

    void readSettings(void);
    void saveSettings(void);
    bool prepareStartProcess(BatchProcessImagesItem *item,
                             const QString& albumDest);

    bool ResizeImage(int &w, int &h, int SizeFactor);

private:

    const static QString RCNAME;
    const static QString RC_GROUP_NAME;

    /**
     * Utility method that fills the type mapping. C defines the command builder
     * to use for the mapping, D the option dialog for this resize type.
     *
     * @param localizedName localized name of the mapping
     */
    template<class C, class D>
    void addResizeType(QString localizedName);

    /**
     * Maps the localized resize type names to their command builders and option
     * dialogs.
     */
    QMap<QString, QPair<ResizeCommandBuilder*, ResizeOptionsBaseDialog*> > m_resizeTypeMapping;

    KIPIPlugins::KPAboutData *m_about;

};

}  // namespace KIPIBatchProcessImagesPlugin

#endif  // RESIZEIMAGESDIALOG_H
