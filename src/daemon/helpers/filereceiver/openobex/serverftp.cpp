
/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
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

#include "serverftp.h"
#include "server_interface.h"
#include "server_session_interface.h"
#include "serversession.h"
#include "filereceiversettings.h"

#include <KDebug>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <kstandarddirs.h>
#include <QtGui/QDesktopServices>
#include <kstandarddirs.h>

OpenObex::ServerFtp::ServerFtp(const QString& addr)
    : QObject(0),  m_dbusServer(0)
{
    kDebug() << "ServerFtp: " << addr;

    QDBusInterface* manager = new QDBusInterface("org.openobex", "/org/openobex",
        "org.openobex.Manager", QDBusConnection::sessionBus());

    FileReceiverSettings::self()->readConfig();

    QList<QVariant> args;
    args << addr << "ftp" << FileReceiverSettings::self()->requirePin();

    kDebug() << args;

    manager->callWithCallback("CreateBluetoothServer", args, this,
        SLOT(serverCreated(QDBusObjectPath)),
        SLOT(serverCreatedError(QDBusError)));

}

OpenObex::ServerFtp::~ServerFtp()
{
    kDebug();
    if (m_dbusServer) {
      m_dbusServer->Stop();
      m_dbusServer->Close();
    }
    disconnect();
    delete m_dbusServer;
}

void OpenObex::ServerFtp::slotErrorOccured(const QString& errorName, const QString& errorMessage)
{
    kDebug() << "error_name" << errorName << "error_message" << errorMessage;
}

void OpenObex::ServerFtp::slotSessionCreated(const QDBusObjectPath &path)
{
    kDebug() << "slotSessionCreated path" << path.path();

    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusConnection = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
    org::openobex::ServerSession* dbusServerSession = new
        org::openobex::ServerSession("org.openobex", path.path(), dbusConnection, this);

    if (!dbusServerSession->isValid()) {
        kDebug() << "invalid org::openobex::ServerSession interface";
        return;
    }
}

void OpenObex::ServerFtp::slotSessionRemoved(const QDBusObjectPath &path)
{
    kDebug() << "path" << path.path();

//     if (d->serverSessions.contains(path.path())) {
//       d->serverSessions[path.path()]->queueDelete();
//       d->serverSessions.remove(path.path());
//     }
}
void OpenObex::ServerFtp::serverCreated(const QDBusObjectPath &path)
{
    m_dbusServer = new org::openobex::Server("org.openobex",
        path.path(), QDBusConnection::sessionBus(), this);

    if (!m_dbusServer->isValid()) {
        kDebug() << "open obex error: invalid dbus server interface" << path.path();
        return;
    }

    kDebug() << "session interface created for: " << m_dbusServer->path();

    connect(m_dbusServer, SIGNAL(SessionCreated(QDBusObjectPath)), this,
        SLOT(slotSessionCreated(QDBusObjectPath)));
    connect(m_dbusServer, SIGNAL(SessionRemoved(QDBusObjectPath)),
        this, SLOT(slotSessionRemoved(QDBusObjectPath)));
    connect(m_dbusServer, SIGNAL(ErrorOccured(const QString&, const QString&)),
        this, SLOT(slotErrorOccured(const QString&, const QString&)));

    kDebug() << "rootFolder: " << FileReceiverSettings::self()->rootFolder().path();
    kDebug() << "allowWrite: " << FileReceiverSettings::self()->allowWrite();

    m_dbusServer->Start(FileReceiverSettings::self()->rootFolder().path(), FileReceiverSettings::self()->allowWrite(), true);
}

void OpenObex::ServerFtp::serverCreatedError(const QDBusError &error)
{
    kDebug() << error.message();
}
