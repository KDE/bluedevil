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
#include "version.h"
#include "obexsession.h"
#include "obexftp/listdirjob.h"

#include "obexd_client.h"

#include <QVariantMap>
#include <QHash>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

#define ENSURE_SESSION_CREATED(address) if (!d->m_sessionMap.contains(address)) { \
        kDebug(dobex()) << "The address " << address << " doesn't has a session"; \
        stablishConnection(address); \
        return; \
    } \
    if (d->m_sessionMap[address]->status() == ObexSession::Connecting) { \
        kDebug(dobex()) << "The session is waiting to be connected"; \
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

    QHash <QString, ObexSession*> m_sessionMap;
    QHash <QObject*, QString> m_sessionMapReverse;

    org::bluez::obex::Client1 *m_client;

    QEventLoop loop;
};

ObexFtpDaemon::ObexFtpDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    d->m_client = 0;
    KAboutData aboutData(
        "obexftpdaemon",
        "bluedevil",
        ki18n("ObexFtp Daemon"),
        bluedevil_version,
        ki18n("ObexFtp Daemon"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "afiestas@kde.org",
        "http://www.afiestas.org");

    connect(Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)),
            this, SLOT(usableAdapterChanged(Adapter*)));

    d->m_status = Private::Offline;
    if (Manager::self()->usableAdapter()) {
        onlineMode();
    }

    qDBusRegisterMetaType<QVariantMapList>();
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
    kDebug(dobex());
    if (d->m_status == Private::Online) {
        kDebug(dobex()) << "Already in onlineMode";
        return;
    }

    d->m_client = new org::bluez::obex::Client1("org.bluez.obex", "/org/bluez/obex", QDBusConnection::sessionBus(), this);

    d->m_status = Private::Online;
}

void ObexFtpDaemon::offlineMode()
{
    kDebug(dobex()) << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug(dobex()) << "Already in offlineMode";
        return;
    }

//     QHash<QString, ObexSession*>::const_iterator i = d->m_sessionMap.constBegin();
//     while (i != d->m_sessionMap.constEnd()) {
//         if (d->m_sessionMap.contains(i.key())) {
//             d->m_sessionMap[i.key()]->Disconnect();
//             d->m_sessionMap[i.key()]->Close();
//             d->m_sessionMap[i.key()]->deleteLater();
//         }
//         d->m_sessionMap.remove(i.key());
//         ++i;
//     }
//
//     delete d->m_manager;
    d->m_status = Private::Offline;
}

void ObexFtpDaemon::usableAdapterChanged(Adapter *adapter)
{
    if (adapter) {
        onlineMode();
    } else {
        offlineMode();
    }
}

void ObexFtpDaemon::stablishConnection(QString dirtyAddress, const QDBusMessage& msg)
{
    QString address = cleanAddress(dirtyAddress);

    kDebug(dobex()) << msg.service() << msg.path();
    kDebug(dobex()) << "Address: " << address;
    if (d->m_status == Private::Offline) {
        kDebug(dobex()) << "We're offline, so do nothing";
        QDBusMessage error = msg.createErrorReply("Offline", i18n("No Bluetooth device found"));
        QDBusConnection::sessionBus().send(error);
        return;
    }

    if (address.isEmpty()) {
        kDebug(dobex()) << "Address is Empty";
        QDBusMessage error = msg.createErrorReply("Invalid address", i18n("Invalid address"));
        QDBusConnection::sessionBus().send(error);
        return;
    }


    //We already have a session for that address
    if (d->m_sessionMap.contains(address)) {
        //But this session is waiting for being connected
        if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
            kDebug(dobex()) << "Session for this address is waiting for being connected";
            d->m_sessionMap[address]->addMessage(msg);
            return;
        }

        //At this point the session should be connected, if not we have done something wrong
        Q_ASSERT(d->m_sessionMap[address]->status() == ObexSession::Connected);

        kDebug(dobex()) << "We already have a session, so do nothing";
        QDBusConnection::sessionBus().send(msg.createReply());
        return;
    }

    kDebug(dobex()) << "Creating a new obexSession";

    ObexSession *session = new ObexSession(address, d->m_client, msg, this);
    connect(session, SIGNAL(destroyed(QObject*)), SLOT(sessionDestroyed(QObject*)));
    d->m_sessionMap[address] = session;
    d->m_sessionMapReverse[session] = address;
}

void ObexFtpDaemon::changeCurrentFolder(QString address, QString path)
{
    kDebug(dobex());
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->ChangeCurrentFolderToRoot().waitForFinished();
//
//     QStringList list = path.split("/");
//     Q_FOREACH(const QString &dir, list) {
//         if (!dir.isEmpty() && dir != address) {
//             kDebug(dobex()) << "Changing to: " << dir;
//             QDBusPendingReply <void > a = d->m_sessionMap[address]->ChangeCurrentFolder(dir);
//             a.waitForFinished();
//             kDebug(dobex())  << "Change Error: " << a.error().message();
//         } else {
//             kDebug(dobex()) << "Skyping" << dir;
//         }
//     }
}

QVariantMapList ObexFtpDaemon::listDir(QString dirtyAddress, QString path, const QDBusMessage& msg)
{
    QString address = cleanAddress(dirtyAddress);
    kDebug(dobex()) << "ROLFMAO !" << address;
    if (!d->m_sessionMap.contains(address)) {
        kDebug(dobex()) << "MA MEN";
        return QVariantMapList();
    }

//     msg.setDelayedReply(true);
//     ListDirJob *job = new ListDirJob(d->m_sessionMap[address], msg, this);
//     job->start();

QVariantMapList rolf;
    QVariantMap map;
    map["meh"] = "rolf";
    rolf.append(map);
    return rolf;
//     QString address = cleanAddress(dirtyAddress);
//     if (!d->m_sessionMap.contains(address)) {
//         kDebug(dobex()) << "The address " << address << " doesn't has a session";
//         stablishConnection(address);
//         return QString();
//     }
//     if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
//         kDebug(dobex()) << "The session is waiting to be connected";
//         return QString();
//     }
//
//     address.replace("-", ":");
//     changeCurrentFolder(address, path);
//
//     d->m_sessionMap[address]->resetTimer();
//     QDBusPendingReply<QString> reply = d->m_sessionMap[address]->RetrieveFolderListing();
//     reply.waitForFinished();
//     if (reply.isError()) {
//         kDebug(dobex()) << reply.error().message();
//         kDebug(dobex()) << reply.error().name();
//         return QString();
//     }
//
//     QString ret = reply.value();
//
//     kDebug(dobex()) << ret;
//
//     return ret;
}

void ObexFtpDaemon::copyRemoteFile(QString dirtyAddress, QString fileName, QString destPath)
{
    kDebug(dobex()) << destPath;
//     QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address);
//
//     KUrl url = KUrl(fileName);
//     changeCurrentFolder(address, url.directory());
//     kDebug(dobex()) << d->m_sessionMap[address]->GetCurrentPath().value();
//     kDebug(dobex()) << url.fileName();
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->CopyRemoteFile(url.fileName(), destPath);
}

void ObexFtpDaemon::sendFile(QString dirtyAddress, QString localPath, QString destPath)
{
    QString address = cleanAddress(dirtyAddress);

//     kDebug(dobex());
//     ENSURE_SESSION_CREATED(address);
//     changeCurrentFolder(address, destPath);
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->SendFile(localPath);
}

void ObexFtpDaemon::createFolder(QString dirtyAddress, QString path)
{
    kDebug(dobex());
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
    kDebug(dobex());
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
    return false;
//     kDebug(dobex());
//     QString address = cleanAddress(dirtyAddress);
//     if (!d->m_sessionMap.contains(address)) {
//         kDebug(dobex()) << "The address " << address << " doesn't has a session";
//         stablishConnection(address);
//         return true;//Fake the busy state, so stablishConneciton can work
//     }
//     if (d->m_sessionMap[address]->status() == ObexSession::Connecting) {
//         kDebug(dobex()) << "The session is waiting to be connected";
//         return true;
//     }
//
//     d->m_sessionMap[address]->resetTimer();
//     return d->m_sessionMap[address]->IsBusy().value();
}

void ObexFtpDaemon::Cancel(QString dirtyAddress)
{
    QString address = cleanAddress(dirtyAddress);
//     ENSURE_SESSION_CREATED(address)
//
//     d->m_sessionMap[address]->resetTimer();
//     d->m_sessionMap[address]->Cancel();
}


void ObexFtpDaemon::SessionConnected(QDBusObjectPath path)
{
    kDebug(dobex()) << "SessionConnected!" << path.path();
/*
    QString address = getAddressFromSession(path.path());

    if (address.isEmpty() || !d->m_sessionMap.contains(address)) {
        kDebug(dobex()) << "This seasson is from another process";
        return;
    }

    d->m_sessionMap[address]->setStatus(ObexSession::Connected);

    connect(d->m_sessionMap[address], SIGNAL(sessionTimeout()), this, SLOT(sessionDisconnected()));
    connect(d->m_sessionMap[address], SIGNAL(Closed()), this, SLOT(sessionDisconnected()));
    connect(d->m_sessionMap[address], SIGNAL(Disconnected()), this, SLOT(sessionDisconnected()));
    connect(d->m_sessionMap[address], SIGNAL(sessionTimeout()), this, SIGNAL(Cancelled()));
    connect(d->m_sessionMap[address], SIGNAL(Closed()), this, SIGNAL(Cancelled()));
    connect(d->m_sessionMap[address], SIGNAL(Disconnected()), this, SIGNAL(Cancelled()));
    connect(d->m_sessionMap[address], SIGNAL(Cancelled()), this, SIGNAL(Cancelled()));
    connect(d->m_sessionMap[address], SIGNAL(TransferCompleted()), this, SIGNAL(transferCompleted()));
    connect(d->m_sessionMap[address], SIGNAL(TransferProgress(qulonglong)), this, SIGNAL(transferProgress(qulonglong)));
    connect(d->m_sessionMap[address], SIGNAL(ErrorOccurred(QString,QString)), this, SIGNAL(errorOccurred(QString,QString)));

    emit sessionConnected(address);*/
}

void ObexFtpDaemon::SessionClosed(QDBusObjectPath path)
{
    kDebug(dobex());
//     QHashIterator<QString, ObexSession*> i(d->m_sessionMap);
//     while (i.hasNext()) {
//         i.next();
//         if (i.value()->path() == path.path()) {
//             kDebug(dobex()) << "Removing : " << i.key();
//             emit sessionClosed(i.key());
//
//             i.value()->deleteLater();
//
//             d->m_sessionMap.remove(i.key());
//             return;
//         }
//     }
//
//     kDebug(dobex()) << "Attempt to remove a nto existing session";
}

void ObexFtpDaemon::sessionDisconnected()
{
    kDebug(dobex()) << "Session disconnected";
//     ObexSession* session =  static_cast <ObexSession*>(sender());
//     kDebug(dobex()) << session->path();
//     kDebug(dobex()) << session->status();
//
//     d->m_sessionMap.remove(d->m_sessionMap.key(session));
//     delete session;
}

void ObexFtpDaemon::sessionDestroyed(QObject* obj)
{
    kDebug(dobex()) << obj;
    QString address = d->m_sessionMapReverse.take(obj);
    d->m_sessionMap.remove(address);
}

QString ObexFtpDaemon::getAddressFromSession(QString path)
{
    kDebug(dobex()) << path;
//     QStringMap info = d->m_manager->GetSessionInfo(QDBusObjectPath(path)).value();
//     return info["BluetoothTargetAddress"];
    return QString();
}

QString ObexFtpDaemon::cleanAddress(QString& dirtyAddress) const
{
    dirtyAddress.replace("-", ":");
    return dirtyAddress.toLower();
}

extern int dobex() { static int s_area = KDebug::registerArea("ObexDaemon", false); return s_area; }