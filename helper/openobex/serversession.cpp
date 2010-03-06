/*
    This file is part of the KDE project

    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "serversession.h"
#include "filetransferjob.h"
#include <QtDBus>
#include <KDebug>

#include <kstatusbarjobtracker.h>
#include <KIO/JobUiDelegate>

using namespace OpenObex;

ServerSession::ServerSession(const QString& path, const QString& bluetoothAddress): QObject(0)
{
    m_path = path;
    m_bluetoothAddress = bluetoothAddress;
    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusConnection = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
    m_dbusServerSession = new org::openobex::ServerSession("org.openobex", path, dbusConnection,
        this);

    connect(m_dbusServerSession, SIGNAL(Cancelled()),
        this, SLOT(slotCancelled()));
    connect(m_dbusServerSession, SIGNAL(Disconnected()),
        this, SLOT(slotDisconnected()));
    connect(m_dbusServerSession, SIGNAL(TransferStarted(const QString&, const QString&, qulonglong)),
        this, SLOT(slotTransferStarted(const QString&, const QString&, qulonglong)));
}

ServerSession::~ServerSession()
{
    kDebug();
    disconnect();
    m_dbusServerSession->deleteLater();
}

void ServerSession::accept()
{
    m_dbusServerSession->Accept();
}

void ServerSession::reject()
{
    m_dbusServerSession->Reject();
}

QString ServerSession::path()
{
    return m_path;
}


void ServerSession::cancel()
{
    m_dbusServerSession->Cancel();
}


void ServerSession::slotCancelled()
{
    qDebug() << "slotCancelled()";
}

void ServerSession::slotDisconnected()
{
    qDebug() << "slotDisconnected()";
}

QString ServerSession::bluetoothAddress()
{
  return m_bluetoothAddress;
}

org::openobex::ServerSession* ServerSession::dbusServerSession()
{
    return m_dbusServerSession;
}

void ServerSession::slotTransferStarted(const QString& filename, const QString& localPath,
    qulonglong totalBytes)
{
    qDebug() << "slotTransferStarted()" << "filename" << filename << "localPath" << localPath <<
        "totalBytes" << totalBytes;
    FileTransferJob *fileTransferJob = new FileTransferJob(this, filename, localPath, totalBytes);
    KIO::getJobTracker()->registerJob(fileTransferJob);
    fileTransferJob->start();
}
