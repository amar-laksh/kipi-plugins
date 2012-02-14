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

#include "actionthreadbase.moc"

// Qt includes

#include <QMutexLocker>
#include <QWaitCondition>

// KDE includes

#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>
#include <ThreadWeaver/Weaver>
#include <threadweaver/Job.h>
#include <solid/device.h>

using namespace ThreadWeaver;

namespace KIPIPlugins
{

class ActionThreadBase::ActionThreadBasePriv
{
public:

    ActionThreadBasePriv()
    {
        running       = false;
        weaverRunning = false;
        weaver        = 0;
        log           = 0;
    }

    bool                  running;
    bool                  weaverRunning;

    QWaitCondition        condVarJobs;

    ThreadWeaver::Weaver* weaver;
    KPWeaverObserver*     log;
};

ActionThreadBase::ActionThreadBase(QObject* parent)
    : QThread(parent), d(new ActionThreadBasePriv)
{
    const int maximumNumberOfThreads = qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
    d->log                           = new KPWeaverObserver(this);
    d->weaver                        = new ThreadWeaver::Weaver(this);
    d->weaver->registerObserver(d->log);
    d->weaver->setMaximumNumberOfThreads(maximumNumberOfThreads);
    kDebug() << "Starting Main Thread";
}

ActionThreadBase::~ActionThreadBase()
{
    kDebug() << "calling action thread destructor";
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    delete d->log;
    delete d;
}

void ActionThreadBase::slotFinished()
{
    kDebug() << "Finish Main Thread";
    d->weaverRunning = false;
    d->condVarJobs.wakeAll();
}

void ActionThreadBase::cancel()
{
    kDebug() << "Cancel Main Thread";
    QMutexLocker lock(&m_mutex);
    m_todo.clear();
    d->running       = false;
    d->weaverRunning = true;
    d->weaver->dequeue();
    m_condVar.wakeAll();
    d->condVarJobs.wakeAll();
}

void ActionThreadBase::run()
{
    d->running       = true;
    d->weaverRunning = false;
    kDebug() << "In action thread Run";

    while (d->running)
    {
        JobCollection* t = 0;
        {
            QMutexLocker lock(&m_mutex);
            if (!m_todo.isEmpty())
            {
                if (!d->weaverRunning)
                {
                    t = m_todo.takeFirst();
                }
                else
                {
                    d->condVarJobs.wait(&m_mutex);
                }
            }
            else
            {
                m_condVar.wait(&m_mutex);
            }
        }

        if (t)
        {
            connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                    this, SLOT(slotFinished()));

            connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                    t, SLOT(deleteLater()));

            d->weaver->enqueue(t);
            d->weaverRunning = true;
        }
    }

    d->weaver->finish();
    kDebug() << "Exiting Action Thread";
}

}  // namespace KIPIPlugins
