/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : re-implementation of action thread using threadweaver
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ACTIONTHREADBASE_H
#define ACTIONTHREADBASE_H

// Qt includes

#include <QList>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// KDE includes

#include <kurl.h>
#include <ThreadWeaver/JobCollection>

//Local includes

#include "kpweaverobserver.h"
#include "kipiplugins_export.h"

namespace KIPIPlugins
{

class KIPIPLUGINS_EXPORT ActionThreadBase : public QThread
{
    Q_OBJECT

public:

    ActionThreadBase(QObject* parent=0);
    ~ActionThreadBase();

    void cancel();

protected:

    void run();

private Q_SLOTS:

    void slotFinished();

protected:

    QWaitCondition                      m_condVar;
    QMutex                              m_mutex;
    QList<ThreadWeaver::JobCollection*> m_todo;

private:

    class ActionThreadBasePriv;
    ActionThreadBasePriv* const d;
};

}  // namespace KIPIPlugins

#endif // ACTIONTHREAD_H
