/***************************************************************************
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

#include "ObexFtpDaemon.h"
#include "obexdmanager.h"
#include "obexdclient.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QVariantMap>
#include <QHash>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

#define ENSURE_SESSION_CREATED(address) if (!d->m_sessionMap.contains(address)) { \
        kDebug() << "The address " << address << " doesn't has a session"; \
        stablishConnection(address); \
        return; \
    } \
    if (d->m_sessionMap[address]->status() == ObexSession::Connecting) { \
        kDebug() << "The session is waiting to be connected"; \
        return; \
    }

using namespace BlueDevil;
K_PLUGIN_FACTORY(ObexFtpFactory,
                 registerPlugin<ObexFtpDaemon>();)
K_EXPORT_PLUGIN(ObexFtpFactory("obexftpdaemon", "obexftpdaemon"))

struct ObexFtpDaemon::Private
{
    enum Status {
        Online = 0,
        Offline
    } m_status;

//     QHash <QString, ObexSession*> m_sessionMap;

    org::openobex::Manager  *m_manager;
    org::openobex::Client   *m_client;
};

ObexFtpDaemon::ObexFtpDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    KAboutData aboutData(
        "obexftpdaemon",
        "bluedevil",
        ki18n("ObexFtp Daemon"),
        "1.0",
        ki18n("ObexFtp Daemon"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "alex@ufocoders.com",
        "http://www.afiestas.org");

    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)),
            this, SLOT(defaultAdapterChanged(Adapter*)));

    d->m_status = Private::Offline;
    if (Manager::self()->defaultAdapter()) {
        onlineMode();
    }
}

ObexFtpDaemon::~ObexFtpDaemon()
{
    if (d->m_status == Private::Online) {
        offlineMode();
    }
    delete d;
}

void ObexFtpDaemon::onlineMode()
{
    kDebug();
    if (d->m_status == Private::Online) {
        kDebug() << "Already in onlineMode";
        return;
    }

    d->m_manager = new org::openobex::Manager("org.openobex", "/", QDBusConnection::sessionBus());
    connect(d->m_manager, SIGNAL(SessionCreated(QDBusObjectPath)), this, SLOT(SessionCreated(QDBusObjectPath)));
    connect(d->m_manager, SIGNAL(SessionRemoved(QDBusObjectPath)), this, SLOT(SessionRemoved(QDBusObjectPath)));

    d->m_client = new org::openobex::Client("org.openobex.client", "/", QDBusConnection::sessionBus());
    d->m_status = Private::Online;
}

void ObexFtpDaemon::offlineMode()
{
    kDebug() << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug() << "Already in offlineMode";
        return;
    }

//     QHash<QString, ObexSession*>::const_iterator i = d->m_sessionMap.constBegin();
//     while (i != d->m_sessionMap.constEnd()) {
//         if (d->m_sessionMap[i.key()]) {
//             d->m_sessionMap[i.key()]->Disconnect().waitForFinished();
//             d->m_sessionMap[i.key()]->Close().waitForFinished();
//             delete d->m_sessionMap[i.key()];
//         }
//         d->m_sessionMap.remove(i.key());
//         ++i;
//     }

    delete d->m_manager;
    delete d->m_client;
    d->m_status = Private::Offline;
}

void ObexFtpDaemon::defaultAdapterChanged(Adapter *adapter)
{
    if (adapter) {
        onlineMode();
    } else {
        offlineMode();
    }
}

void ObexFtpDaemon::stablishConnection(QString dirtyAddress)
{
    QString address = cleanAddress(dirtyAddress);

    kDebug() << "Address: " << address;
    if (d->m_status == Private::Offline) {
        kDebug() << "We're offline, so do nothing";
        return;
    }

    if (address.isEmpty()) {
        kDebug() << "Address is Empty";
    }

    //We already have a session for that address
//     if (d->m_sessionMap.contains(address)) {
//         //But this session is waiting for being connected
//         if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
//             kDebug() << "Session for this address is waiting for being connected";
//             return;
//         }
//
//         kDebug() << "We already have a session, so do nothing";
//         emit sessionConnected(address);
//         return;
//     }

    kDebug() << "Telling to the manager to create the session";

    QVariantMap args;
    args["Destination"] = QVariant(address);
    args["Target"] = QVariant("ftp");

    QDBusPendingReply <QDBusObjectPath > rep = d->m_client->CreateSession(args);
    kDebug() << "Valid: " << rep.isValid();
    kDebug() << "Error: " << rep.isError();
    kDebug() << "errir: " << rep.error().message();
    kDebug() << "Path: " << rep.value().path();
}

void ObexFtpDaemon::changeCurrentFolder(QString address, QString path)
{
    kDebug();
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->ChangeCurrentFolderToRoot().waitForFinished();
//
//     QStringList list = path.split("/");
//     Q_FOREACH(const QString &dir, list) {
//         if (!dir.isEmpty() && dir != address) {
//             kDebug() << "Changing to: " << dir;
//             QDBusPendingReply <void > a = d->m_sessionMap[address]->ChangeCurrentFolder(dir);
//             a.waitForFinished();
//             kDebug()  << "Change Error: " << a.error().message();
//         } else {
//             kDebug() << "Skyping" << dir;
//         }
//     }
}

QString ObexFtpDaemon::listDir(QString dirtyAddress, QString path)
{
//     kDebug();
//     QString address = cleanAddress(dirtyAddress);
//     if (!d->m_sessionMap.contains(address)) {
//         kDebug() << "The address " << address << " doesn't has a session";
//         stablishConnection(address);
//         return QString();
//     }
//     if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
//         kDebug() << "The session is waiting to be connected";
//         return QString();
//     }
//
//     address.replace("-", ":");
//     changeCurrentFolder(address, path);
//
//     d->m_sessionMap[address]->resetTimer();
//     QString ret = d->m_sessionMap[address]->RetrieveFolderListing().value();
//
//     kDebug() << ret;
//
//     return ret;
    return QString();
}

void ObexFtpDaemon::copyRemoteFile(QString dirtyAddress, QString fileName, QString destPath)
{
//     kDebug() << destPath;
//     QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address);
//
//     KUrl url = KUrl(fileName);
//     changeCurrentFolder(address, url.directory());
//     kDebug() << d->m_sessionMap[address]->GetCurrentPath().value();
//     kDebug() << url.fileName();
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->CopyRemoteFile(url.fileName(), destPath);
}

void ObexFtpDaemon::sendFile(QString dirtyAddress, QString localPath, QString destPath)
{
//     QString address = cleanAddress(dirtyAddress);
//
//     kDebug();
//     ENSURE_SESSION_CREATED(address);
//     changeCurrentFolder(address, destPath);
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->SendFile(localPath);
}

void ObexFtpDaemon::createFolder(QString dirtyAddress, QString path)
{
//     kDebug();
//     QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address);
//
//     KUrl url(path);
//     changeCurrentFolder(address, url.directory());
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->CreateFolder(url.fileName()).waitForFinished();
}

void ObexFtpDaemon::deleteRemoteFile(QString dirtyAddress, QString path)
{
//     kDebug();
//     QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address);
//
//     KUrl url(path);
//     changeCurrentFolder(address, url.directory());
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->DeleteRemoteFile(url.fileName()).waitForFinished();;
}

bool ObexFtpDaemon::isBusy(QString dirtyAddress)
{
//     kDebug();
//     QString address = cleanAddress(dirtyAddress);
//     if (!d->m_sessionMap.contains(address)) {
//         kDebug() << "The address " << address << " doesn't has a session";
//         stablishConnection(address);
//         return true;//Fake the busy state, so stablishConneciton can work
//     }
//     if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
//         kDebug() << "The session is waiting to be connected";
//         return true;
//     }
//
//     d->m_sessionMap[address]->resetTimer();
//     return d->m_sessionMap[address]->IsBusy().value();
    return false;
}

void ObexFtpDaemon::Cancel(QString dirtyAddress)
{
//     QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address)
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->Cancel();
}

<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
        <interface name="org.freedesktop.DBus.Introspectable">
                <method name="Introspect">
                        <arg type="s" direction="out"/>
                </method>
        </interface>
        <interface name="org.openobex.Session">
                <method name="GetProperties">
                        <arg type="a{sv}" direction="out"/>
                </method>
                <method name="AssignAgent">
                        <arg type="o" direction="in"/>
                </method>
                <method name="ReleaseAgent">
                        <arg type="o" direction="in"/>
                </method>
        </interface>
        <interface name="org.openobex.FileTransfer">
                <method name="ChangeFolder">
                        <arg type="s" direction="in"/>
                </method>
                <method name="CreateFolder">
                        <arg type="s" direction="in"/>
                </method>
                <method name="ListFolder">
                        <arg type="aa{sv}" direction="out"/>
                </method>
                <method name="GetFile">
                        <arg type="s" direction="in"/>
                        <arg type="s" direction="in"/>
                </method>
                <method name="PutFile">
                        <arg type="s" direction="in"/>
                        <arg type="s" direction="in"/>
                </method>
                <method name="CopyFile">
                        <arg type="s" direction="in"/>
                        <arg type="s" direction="in"/>
                </method>
                <method name="MoveFile">
                        <arg type="s" direction="in"/>
                        <arg type="s" direction="in"/>
                </method>
                <method name="Delete">
                        <arg type="s" direction="in"/>
                </method>
        </interface>
</node>
void ObexFtpDaemon::SessionCreated(QDBusObjectPath path)
{
    kDebug() << "SessionConnected!" << path.path();

//     QString address = getAddressFromSession(path.path());
//
//     d->m_sessionMap[address]->setStatus(ObexSession::Connected);
//
//     connect(d->m_sessionMap[address], SIGNAL(sessionTimeout()), this, SLOT(sessionDisconnected()));
//     connect(d->m_sessionMap[address], SIGNAL(Closed()), this, SLOT(sessionDisconnected()));
//     connect(d->m_sessionMap[address], SIGNAL(Disconnected()), this, SLOT(sessionDisconnected()));
//     connect(d->m_sessionMap[address], SIGNAL(sessionTimeout()), this, SIGNAL(Cancelled()));
//     connect(d->m_sessionMap[address], SIGNAL(Closed()), this, SIGNAL(Cancelled()));
//     connect(d->m_sessionMap[address], SIGNAL(Disconnected()), this, SIGNAL(Cancelled()));
//     connect(d->m_sessionMap[address], SIGNAL(Cancelled()), this, SIGNAL(Cancelled()));
//     connect(d->m_sessionMap[address], SIGNAL(TransferCompleted()), this, SIGNAL(transferCompleted()));
//     connect(d->m_sessionMap[address], SIGNAL(TransferProgress(qulonglong)), this, SIGNAL(transferProgress(qulonglong)));
//     connect(d->m_sessionMap[address], SIGNAL(ErrorOccurred(QString,QString)), this, SIGNAL(errorOccurred(QString,QString)));
//
//     emit sessionConnected(address);
}

void ObexFtpDaemon::SessionRemoved(QDBusObjectPath path)
{
//     kDebug();
//     QHash<QString, ObexSession*>::const_iterator i = d->m_sessionMap.constBegin();
//     while (i != d->m_sessionMap.constEnd()) {
//         //If the session is connected, so not 0
//         if (i.value()->path() == path.path()) {
//             kDebug() << "Removing : " << i.key();
//             emit sessionClosed(i.key());
//             d->m_sessionMap.remove(i.key());
//             delete i.value();
//             return;
//         }
//         ++i;
//     }
//
//     kDebug() << "Attempt to remove a nto existing session";
}

void ObexFtpDaemon::sessionDisconnected()
{
//     kDebug() << "Session disconnected";
//     ObexSession* session =  static_cast <ObexSession*>(sender());
//     kDebug() << session->path();
//     kDebug() << session->status();
//
//     d->m_sessionMap.remove(d->m_sessionMap.key(session));
//     delete session;
}

QString ObexFtpDaemon::getAddressFromSession(QString path)
{
//     kDebug() << path;
//     QStringMap info = d->m_manager->GetSessionInfo(QDBusObjectPath(path)).value();
//     return info["BluetoothTargetAddress"];
    return QString();
}

QString ObexFtpDaemon::cleanAddress(QString& dirtyAddress) const
{
    dirtyAddress.replace("-", ":");
    return dirtyAddress.toLower();
}
