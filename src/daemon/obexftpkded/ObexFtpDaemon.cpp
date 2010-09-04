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
#include "obexftpmanager.h"
#include "obexftpsession.h"

#include <QVariantMap>
#include <QHash>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>
#include <kfileplacesmodel.h>
#include <kprocess.h>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

#define ENSURE_SESSION_CREATED(address) if (!d->m_sessionMap.contains(address)) { \
        kDebug() << "The address " << address << " doesn't has a session"; \
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

    QHash <QString, org::openobex::Session*> m_sessionMap;

    org::openobex::Manager  *m_manager;

    QEventLoop loop;
};

ObexFtpDaemon::ObexFtpDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    KAboutData aboutData(
        "obexftpdaemon",
        "obexftpdaemon",
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

    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<QStringMap>("QStringMap");
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

    d->m_manager = new org::openobex::Manager("org.openobex", "/org/openobex", QDBusConnection::sessionBus(), 0);

    d->m_status = Private::Online;
}

void ObexFtpDaemon::offlineMode()
{
    kDebug() << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug() << "Already in offlineMode";
        return;
    }

    QHash<QString, org::openobex::Session*>::const_iterator i = d->m_sessionMap.constBegin();
    while (i != d->m_sessionMap.constEnd()) {
        d->m_sessionMap[i.key()]->Disconnect().waitForFinished();
        d->m_sessionMap[i.key()]->Close().waitForFinished();
        delete d->m_sessionMap[i.key()];
        d->m_sessionMap.remove(i.key());
    }

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

void ObexFtpDaemon::stablishConnection(QString address)
{
    address.replace("-",":");

    kDebug() << "Address: " << address;
    if (d->m_status == Private::Offline) {
        kDebug() << "We're offline, so do nothing";
        return;
    }

    if (address.isEmpty()) {
        kDebug() << "Address is Empty";
    }

    //We already have a session for that address
    if (d->m_sessionMap.contains(address)) {
        kDebug() << "We already have a session, so do nothing";
        emit sessionConnected(address);
        return;
    }

    connect(d->m_manager, SIGNAL(SessionConnected(QDBusObjectPath)), this, SLOT(SessionConnected(QDBusObjectPath)));
    connect(d->m_manager, SIGNAL(SessionClosed(QDBusObjectPath)), this, SLOT(SessionClosed(QDBusObjectPath)));
    QDBusPendingReply <QDBusObjectPath > rep = d->m_manager->CreateBluetoothSession(address, "00:00:00:00:00:00", "ftp");

    kDebug() << "Path: " << rep.value().path();
}

void ObexFtpDaemon::changeCurrentFolder(QString address, QString path)
{
    d->m_sessionMap[address]->ChangeCurrentFolderToRoot().waitForFinished();

    QStringList list = path.split("/");
    Q_FOREACH(const QString &dir, list) {
        if (!dir.isEmpty() && dir != address) {
            kDebug() << "Changing to: " << dir;
            QDBusPendingReply <void > a = d->m_sessionMap[address]->ChangeCurrentFolder(dir);
            a.waitForFinished();
            kDebug()  << "Change Error: " << a.error().message();
        } else {
            kDebug() << "Skyping" << dir;
        }
    }
}

QString ObexFtpDaemon::listDir(QString address, QString path)
{
    address.replace("-", ":");
    if (!d->m_sessionMap.contains(address)) {
        kDebug() << "The address " << address << " doesn't has a session";
        return QString();
    }

    address.replace("-", ":");
    changeCurrentFolder(address, path);

    QString ret = d->m_sessionMap[address]->RetrieveFolderListing().value();

    kDebug() << ret;

    return ret;
}

void ObexFtpDaemon::copyRemoteFile(QString address, QString fileName, QString destPath)
{
    kDebug();
    address.replace("-", ":");
    ENSURE_SESSION_CREATED(address);

    KUrl url = KUrl(fileName);
    changeCurrentFolder(address, url.directory());
    kDebug() << d->m_sessionMap[address]->GetCurrentPath().value();
    kDebug() << url.fileName();
    d->m_sessionMap[address]->CopyRemoteFile(url.fileName(), destPath);
}

void ObexFtpDaemon::sendFile(QString address, QString localPath, QString destPath)
{
    address.replace("-", ":");

    kDebug();
    ENSURE_SESSION_CREATED(address);
    changeCurrentFolder(address, destPath);
    d->m_sessionMap[address]->SendFile(localPath);
}

void ObexFtpDaemon::createFolder(QString address, QString path)
{
    kDebug();
    address.replace("-", ":");
    ENSURE_SESSION_CREATED(address);

    KUrl url(path);
    changeCurrentFolder(address, url.directory());

    d->m_sessionMap[address]->CreateFolder(url.fileName()).waitForFinished();
}

void ObexFtpDaemon::deleteRemoteFile(QString address, QString path)
{
    kDebug();
    address.replace("-", ":");
    ENSURE_SESSION_CREATED(address);

    KUrl url(path);
    changeCurrentFolder(address, url.directory());

    d->m_sessionMap[address]->DeleteRemoteFile(url.fileName()).waitForFinished();;
}

void ObexFtpDaemon::SessionConnected(QDBusObjectPath path)
{
    kDebug() << "SessionConnected!" << path.path();

    org::openobex::Session  *session = new org::openobex::Session("org.openobex", path.path(), QDBusConnection::sessionBus(), 0);
    QString address = getAddressFromSession(path.path());
    d->m_sessionMap.insert(address, session);

    connect(session, SIGNAL(TransferCompleted()), this, SIGNAL(transferCompleted()));
    connect(session, SIGNAL(TransferProgress(qulonglong)), this, SIGNAL(transferProgress(qulonglong)));
    connect(session, SIGNAL(ErrorOccurred(QString,QString)), this, SIGNAL(errorOccurred(QString,QString)));

    emit sessionConnected(address);
}

void ObexFtpDaemon::SessionClosed(QDBusObjectPath path)
{
    QHash<QString, org::openobex::Session*>::const_iterator i = d->m_sessionMap.constBegin();
    while (i != d->m_sessionMap.constEnd()) {
        if (i.value()->path() == path.path()) {
            kDebug() << "Removing : " << i.key();
            d->m_sessionMap.remove(i.key());
            return;
        }
    }

    kDebug() << "Attempt to remove a nto existing session";
}

QString ObexFtpDaemon::getAddressFromSession(QString path)
{
    kDebug() << path;
    QStringMap info = d->m_manager->GetSessionInfo(QDBusObjectPath(path)).value();
    return info["BluetoothTargetAddress"];
}