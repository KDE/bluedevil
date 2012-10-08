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

#include "server.h"
#include "server_interface.h"
#include "server_session_interface.h"
#include "serversession.h"
#include "filereceiversettings.h"

#include <KDebug>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>
#include <QtCore/QDir>
#include <QtGui/QDesktopServices>

struct OpenObex::Server::Private
{
    org::openobex::Server* dbusServer;

    /**
     * Array of with key being the ServerSession path and value being the corresponding ServerSession.
     * This allows us to have more than one transfer at a time.
     */
    QMap<QString, OpenObex::ServerSession*> serverSessions;
};

OpenObex::Server::Server(const QString& addr)
    : QObject(0), d(new Private)
{
    kDebug() << "Server: " << addr;

    d->dbusServer = 0;

    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

    //Dbus must be present and this connection can't fail
    if (!dbusconn.isConnected()) {
        return;
    }

    QDBusInterface* manager = new QDBusInterface("org.openobex", "/org/openobex",
        "org.openobex.Manager", dbusconn);

    QString pattern = "opp";
    QList<QVariant> args;
    args << addr << pattern << false;

    manager->callWithCallback("CreateBluetoothServer", args, this,
        SLOT(serverCreated(QDBusObjectPath)),
        SLOT(serverCreatedError(QDBusError)));
    delete dbus;
}

OpenObex::Server::~Server()
{
    kDebug();
    // If serverCreatedError() was called instead of serverCreated(), d->dbusServer won't
    // have been initialized so it will not exist
    if (d->dbusServer) {
      d->dbusServer->Stop();
      d->dbusServer->Close();
    }
    disconnect();
    delete d->dbusServer;
    Q_FOREACH (OpenObex::ServerSession* serverSession, d->serverSessions) {
      serverSession->queueDelete();
    }
    delete d;
}

void OpenObex::Server::slotErrorOccured(const QString& errorName, const QString& errorMessage)
{
    kDebug() << "error_name" << errorName << "error_message" << errorMessage;
}

void OpenObex::Server::slotSessionCreated(QDBusObjectPath path)
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

    QMap<QString, QString> sessionInfo = getServerSessionInfo(path);

    d->serverSessions[path.path()] = new OpenObex::ServerSession(path.path(),
        sessionInfo["BluetoothAddress"]);

}

void OpenObex::Server::slotSessionRemoved(QDBusObjectPath path)
{
    kDebug() << "path" << path.path();

    if (d->serverSessions.contains(path.path())) {
      d->serverSessions[path.path()]->queueDelete();
      d->serverSessions.remove(path.path());
    }
}
void OpenObex::Server::serverCreated(QDBusObjectPath path)
{
    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

    d->dbusServer = new org::openobex::Server("org.openobex",
        path.path(), dbusconn, this);

    if (!d->dbusServer->isValid()) {
        kDebug() << "open obex error: invalid dbus server interface" << path.path();
        return;
    }
    kDebug() << "session interface created for: " << d->dbusServer->path();

    connect(d->dbusServer, SIGNAL(SessionCreated(QDBusObjectPath)), this,
        SLOT(slotSessionCreated(QDBusObjectPath)));
    connect(d->dbusServer, SIGNAL(SessionRemoved(QDBusObjectPath)),
        this, SLOT(slotSessionRemoved(QDBusObjectPath)));
    connect(d->dbusServer, SIGNAL(ErrorOccured(const QString&, const QString&)),
        this, SLOT(slotErrorOccured(const QString&, const QString&)));

    FileReceiverSettings::self()->readConfig();

    bool autoAccept = false;
    if (FileReceiverSettings::autoAccept() == 2) {
        autoAccept = true;
    }

    kDebug() << "AutoAccept: " << autoAccept;
    kDebug() << "SaveUrl: " << FileReceiverSettings::saveUrl().path();

    checkDestinationDir();

    kDebug() << "Final path: " << FileReceiverSettings::saveUrl().path();
    d->dbusServer->Start(FileReceiverSettings::saveUrl().path(), true, autoAccept);
}

void OpenObex::Server::checkDestinationDir()
{
    QFileInfo dir(FileReceiverSettings::saveUrl().path());
    if (dir.exists() && dir.isReadable() && dir.isWritable()) {
        return;
    }

    if (QDir().mkdir(dir.path())) {
        return;
    }

    kDebug() << "Couldn't create dir or it is not readable, fallbacking";
    kDebug() << dir.path();
    kDebug() << QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

    FileReceiverSettings::setSaveUrl(KUrl(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));

    QDir().mkdir(FileReceiverSettings::saveUrl().path());
}

void OpenObex::Server::serverCreatedError(QDBusError error)
{
    kDebug() << error.message();
}


typedef QMap<QString, QString> StringMapReply;
Q_DECLARE_METATYPE(StringMapReply)

QMap<QString,QString> OpenObex::Server::getServerSessionInfo(QDBusObjectPath path)
{
  qDBusRegisterMetaType<StringMapReply>();
  QList<QVariant> argumentList;
  argumentList << QVariant::fromValue(path);
  QDBusReply<QMap<QString, QString> > reply = d->dbusServer->callWithArgumentList(QDBus::Block, QLatin1String("GetServerSessionInfo"), argumentList);
  if (reply.isValid()) {
    return reply.value();
  } else {
    kDebug() << "reply not valid" << reply.error().message();
    return QMap<QString,QString>();
  }
}
