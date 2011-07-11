/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.kipi-plugins.org">http://www.kipi-plugins.org</a>
 *
 * @date   2009-11-21
 * @brief  kipi host test application
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef KIPIIMAGEINFOSHARED_H
#define KIPIIMAGEINFOSHARED_H

// libkipi includes

#include <libkipi/imageinfoshared.h>

class KipiImageInfoShared : public KIPI::ImageInfoShared
{
public:

    KipiImageInfoShared(KIPI::Interface* interface, const KUrl& url);
    ~KipiImageInfoShared();

    QString description();
    void setDescription(const QString&);

    QMap<QString, QVariant> attributes();
    void addAttributes(const QMap<QString, QVariant>& attributesToAdd);
    void delAttributes(const QStringList& attributesToDelete);
    void clearAttributes();

    virtual QDateTime time(KIPI::TimeSpec timeSpec);

private:

    class KipiImageInfoSharedPrivate;
    KipiImageInfoSharedPrivate* const d;
};

#endif // KIPIIMAGEINFOSHARED_H
