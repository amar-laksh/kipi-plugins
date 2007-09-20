/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-19-09
 * Description : Kipi-Plugins shared library.
 *               Interface to write image data to common picture format.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KIPI_WRITE_IMAGE_H
#define KIPI_WRITE_IMAGE_H

// C++ includes.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

// C Ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <jpeglib.h>
#include <tiffio.h>
#include <tiffvers.h>
#include "iccjpeg.h"
#include <png.h>
}

// Libkipi includes.

#include <libkipi/libkipi_export.h>

// LibKDcraw includes.

#include <libkdcraw/rawdecodingsettings.h>

// LibKExiv2 includes. 

#include <libkexiv2/kexiv2.h>

namespace KIPIPlugins
{
class KPWriteImagePriv;

class LIBKIPI_EXPORT KPWriteImage
{
public:
 
    KPWriteImage();
    ~KPWriteImage();

    void setImageData(const QByteArray& data, uint width, uint height, 
                      bool sixteenBit, bool hasAlpha, 
                      const QByteArray& iccProfile,
                      const KExiv2Iface::KExiv2& metadata);

    void setCancel(bool* cancel);
    bool cancel() const;

    bool write2JPEG(const QString& destPath);
    bool write2PNG(const QString& destPath);
    bool write2TIFF(const QString& destPath);

    static QByteArray getICCProfilFromFile(KDcrawIface::RawDecodingSettings::OutputColorSpace colorSpace);

private:
    
    void   writeRawProfile(png_struct *ping, png_info *ping_info, char *profile_type, 
                           char *profile_data, png_uint_32 length);

    size_t concatenateString(char *destination, const char *source, const size_t length);
    size_t copyString(char *destination, const char *source, const size_t length);
    long   formatString(char *string, const size_t length, const char *format,...);
    long   formatStringList(char *string, const size_t length, const char *format, va_list operands);

    void tiffSetExifAsciiTag(TIFF* tif, ttag_t tiffTag, 
                             const KExiv2Iface::KExiv2& metadata, 
                             const char* exifTagName);
    void tiffSetExifDataTag(TIFF* tif, ttag_t tiffTag, 
                            const KExiv2Iface::KExiv2& metadata, 
                            const char* exifTagName);

private: 

    KPWriteImagePriv* d;
};

}  // NameSpace KIPIPlugins

#endif /* KIPI_WRITE_IMAGE_H */
