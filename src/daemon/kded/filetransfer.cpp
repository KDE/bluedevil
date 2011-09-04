/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <afiestas@kde.org>      *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "filetransfer.h"
#include "obexdagent.h"
#include "receivejob.h"

#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>

#include <KDebug>
#include <kio/global.h>
#include <kjobtrackerinterface.h>

FileTransfer::FileTransfer(QObject* parent): QObject(parent)
{
    m_agent = new ObexdAgent(this);
    m_manager = new org::openobex::Manager("org.openobex", "/", QDBusConnection::sessionBus(), this);

    QDBusPendingReply <void > r = m_manager->RegisterAgent(QDBusObjectPath("/BlueDevil_obexdAgent"));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(r, this);

     connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(initFileTransfer(QDBusPendingCallWatcher*)));
}

void FileTransfer::initFileTransfer(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply <void> reply = *watcher;
     if (reply.isError()) {
         kDebug() << reply.error().name();
         kDebug() << reply.error().message();
         return;
     }
     watcher->deleteLater();

    connect(m_manager, SIGNAL(SessionCreated(QDBusObjectPath)), this, SLOT(SessionCreated(QDBusObjectPath)));
    connect(m_manager, SIGNAL(SessionRemoved(QDBusObjectPath)), this, SLOT(SessionRemoved(QDBusObjectPath)));
    connect(m_manager, SIGNAL(TransferStarted(QDBusObjectPath)), this, SLOT(TransferStarted(QDBusObjectPath)));
    connect(m_manager, SIGNAL(TransferCompleted(QDBusObjectPath,bool)), this, SLOT(TransferCompleted(QDBusObjectPath,bool)));
}


FileTransfer::~FileTransfer()
{
    m_manager->UnregisterAgent(QDBusObjectPath("/BlueDevil_obexdAgent"));
    m_manager->disconnect();
}

void FileTransfer::SessionCreated(QDBusObjectPath )
{
    kDebug() << "Session created!";
}

void FileTransfer::SessionRemoved(QDBusObjectPath )
{
    kDebug() << "Session removed!";
}

void FileTransfer::TransferCompleted(const QDBusObjectPath &path, bool success)
{
    kDebug() << "Transfer Completed";
    kDebug() << path.path();

    if (!m_jobs.contains(path.path())) {
        kDebug() << "Do nothing, we don't have this job...";
        return;
    }

    ReceiveJob *job = m_jobs.value(path.path());
    if (!success) {
        job->failed();
    } else {
        job->completed();
    }
}

void FileTransfer::TransferStarted(const QDBusObjectPath &path)
{
    kDebug() << "Transfer Started";

    const QVariantMap info = m_agent->info();
    ReceiveJob *job = new ReceiveJob(path.path(), info["dest"].toString(), info["from"].toString(), info["length"].toInt(), this);
    KIO::getJobTracker()->registerJob(job);
    job->start();

    m_jobs[path.path()] = job;
    connect(job, SIGNAL(destroyed(QObject*)), this, SLOT(jobDestroyed(QObject*)));
}

void FileTransfer::jobDestroyed(QObject* job)
{
    kDebug() << "Removing from hash: " << job;
    ReceiveJob *receiveJob = static_cast<ReceiveJob *>(job);

    m_jobs.remove(m_jobs.key(receiveJob));
}