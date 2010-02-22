/*  This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
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

#include "server.h"
#include "server_interface.h"
#include "bluedevil_service_interface.h"
#include <kdebug.h>


struct OpenObex::Server::Private
{
    org::openobex::Server *dbusServer;
    org::kde::BlueDevil::Service* service;
};

OpenObex::Server::Server(const QString& addr) : QObject(0), d(new Private)
{
    qDebug();
    d->service = 0;
    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

    //Dbus must be pressent and this connection can't fail
    if(!dbusconn.isConnected()){
        return;
    }

    qDebug() << addr;

    QDBusInterface* manager = new QDBusInterface("org.openobex", "/org/openobex", "org.openobex.Manager", dbusconn);

    QString pattern = "opp";
    bool require_pairing = false;
    QList<QVariant> args;
    args << addr << pattern << require_pairing;
    qDebug() << args;
    qDebug() << "CallWithCallback";
    manager->callWithCallback("CreateBluetoothServer", args, this,
        SLOT(serverCreated(QDBusObjectPath)),
        SLOT(serverCreatedError(QDBusError)));
    delete dbus;
}

OpenObex::Server::~Server()
{
    delete d->service;
    delete d;
}

void OpenObex::Server::start()
{
    d->dbusServer->Start(QString("/tmp"), true, false);
    emit started();
}

void OpenObex::Server::stop()
{
    d->dbusServer->Stop();
    emit stopped();
}

void OpenObex::Server::close()
{
    d->dbusServer->Close();
    emit closed();
}

void OpenObex::Server::slotClosed()
{
    qDebug();
}

bool OpenObex::Server::serviceStarted()
{
    d->service = new org::kde::BlueDevil::Service("org.kde.BlueDevil.Service",
        "/Service", QDBusConnection::sessionBus(), this);

    if ((QString)d->service->ping() == "pong") {
        qDebug() << "org::kde::BlueDevil::Service is up and running!";
        return true;
    } else {
        qDebug() << d->service->ping();
        return false;
    }
}

void OpenObex::Server::slotErrorOccured(const QString& errorName, const QString& errorMessage)
{
    qDebug() << "error_name" << errorName << "error_message" << errorMessage;

    if (!serviceStarted()) {
        return;
    }
    d->service->errorOccured(errorName, errorMessage);
}

void OpenObex::Server::slotSessionCreated(QDBusObjectPath path)
{
    qDebug() << "path" << path.path();

    if (!serviceStarted()) {
        return;
    }
    d->service->sessionCreated(path);
}

void OpenObex::Server::slotSessionRemoved(QDBusObjectPath path)
{
    qDebug() << "path" << path.path();

    if (!serviceStarted()) {
        return;
    }
    d->service->sessionRemoved(path);
}

void OpenObex::Server::slotStarted()
{
    qDebug();
}

void OpenObex::Server::slotStopped()
{
    qDebug();
}

void OpenObex::Server::serverCreated(QDBusObjectPath path)
{
    QDBusConnection* dbus = new QDBusConnection("dbus");
    QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

    d->dbusServer = new org::openobex::Server("org.openobex",
        path.path(), dbusconn, this);

    //This interface MUST be valid too, if not is because openobex have some problem
    if(!d->dbusServer->isValid()){
        qDebug() << "open obex error: invalid dbus server interface" << path.path();
        emit openObexError();
        return;
    }
    qDebug() << "session interface created for: " << d->dbusServer->path();

//  connect the DBus Signal to slots
    connect(d->dbusServer, SIGNAL(Started()), this, SLOT(slotStarted()));
    connect(d->dbusServer, SIGNAL(Stopped()), this, SLOT(slotStopped()));
    connect(d->dbusServer, SIGNAL(Closed()), this, SLOT(slotClosed()));
    connect(d->dbusServer, SIGNAL(SessionCreated(QDBusObjectPath)), this,
        SLOT(slotSessionCreated(QDBusObjectPath)));
    connect(d->dbusServer, SIGNAL(SessionRemoved(QDBusObjectPath)),
        this, SLOT(slotSessionRemoved(QDBusObjectPath)));
    connect(d->dbusServer, SIGNAL(ErrorOccured(const QString&, const QString&)),
        this, SLOT(slotErrorOccured(const QString&, const QString&)));
    start();
}

void OpenObex::Server::serverCreatedError(QDBusError error)
{
    qDebug() << error.message();
}

