/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "serversession.h"
#include "serversessionfiletransfer.h"
#include <QtDBus>

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
    connect(m_dbusServerSession, SIGNAL(ErrorOccurred(const QString&, const QString&)),
        this, SLOT(slotErrorOccurred(const QString&, const QString&)));
}

ServerSession::~ServerSession()
{
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

void ServerSession::slotErrorOccurred(const QString& errorName, const QString& errorMessage)
{
    qDebug() << "slotErrorOccurred()" << "errorName" << errorName << "errorMessage" << errorMessage;
}

void ServerSession::slotTransferStarted(const QString& filename, const QString& localPath,
    qulonglong totalBytes)
{
    qDebug() << "slotTransferStarted()" << "filename" << filename << "localPath" << localPath <<
        "totalBytes" << totalBytes;
    m_fileTransfer = new ServerSessionFileTransfer(this, filename, localPath, totalBytes);
    KIO::getJobTracker()->registerJob(m_fileTransfer);
    m_fileTransfer->start();

}
