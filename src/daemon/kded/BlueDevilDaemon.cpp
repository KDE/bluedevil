/*************************************************************************************
 *  Copyright (C) 2010-2012 by Alejandro Fiestas Olivares <afiestas@kde.org>         *
 *  Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>                      *
 *  Copyright (C) 2010 UFO Coders <info@ufocoders.com>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "BlueDevilDaemon.h"
#include "bluezagent.h"
#include "filereceiversettings.h"
#include "version.h"

#include "filereceiver/filereceiver.h"

#include <QProcess>
#include <QDBusServiceWatcher>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QTimer>
#include <QDebug>
#include <QUrl>

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <kfileplacesmodel.h>
#include <kdirnotify.h>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

using namespace BlueDevil;

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedevil.json",
                           registerPlugin<BlueDevilDaemon>();)

Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(QMapDeviceInfo)

struct BlueDevilDaemon::Private
{
    enum Status {
        Online = 0,
        Offline
    } m_status;

    BluezAgent                      *m_bluezAgent;
    KFilePlacesModel                *m_placesModel;
    Adapter                         *m_adapter;
    QDBusServiceWatcher             *m_monolithicWatcher;
    FileReceiver                    *m_fileReceiver;
    QList <DeviceInfo>               m_discovered;
    QTimer                           m_timer;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    qDBusRegisterMetaType <DeviceInfo> ();
    qDBusRegisterMetaType <QMapDeviceInfo> ();

    d->m_bluezAgent = 0;
    d->m_adapter = 0;
    d->m_placesModel = 0;
    d->m_fileReceiver = 0;
    d->m_monolithicWatcher = new QDBusServiceWatcher(QStringLiteral("org.kde.bluedevilmonolithic")
            , QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this);
    d->m_timer.setInterval(20000);
    d->m_timer.setSingleShot(true);

    KAboutData aboutData(
        QStringLiteral("bluedevildaemon"),
        i18n("Bluetooth Daemon"),
        bluedevil_version,
        i18n("Bluetooth Daemon"),
        KAboutLicense::GPL,
        i18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(i18n("Alejandro Fiestas Olivares"), i18n("Maintainer"),
                        QStringLiteral("afiestas@kde.org"), QStringLiteral("http://www.afiestas.org"));

    aboutData.addAuthor(i18n("Eduardo Robles Elvira"), i18n("Maintainer"),
                        QStringLiteral("edulix@gmail.com"), QStringLiteral("http://blog.edulix.es"));

    aboutData.setProgramIconName(QStringLiteral("preferences-system-bluetooth"));
    aboutData.setComponentName(QStringLiteral("bluedevil"));
    KAboutData::registerPluginData(aboutData);

    connect(d->m_monolithicWatcher, SIGNAL(serviceUnregistered(const QString &)), SLOT(monolithicFinished(const QString &)));

    connect(Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)),
            this, SLOT(usableAdapterChanged(Adapter*)));

    d->m_status = Private::Offline;
    usableAdapterChanged(Manager::self()->usableAdapter());

    if (!Manager::self()->adapters().isEmpty()) {
        executeMonolithic();
    }
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    if (d->m_status == Private::Online) {
        offlineMode();
    }

    delete d;
}

bool BlueDevilDaemon::isOnline()
{
    if (d->m_status == Private::Offline) {
        return false;
    }
    return true;
}

QMapDeviceInfo BlueDevilDaemon::knownDevices()
{
    QMapDeviceInfo devices;

    QList <Device* > list = Manager::self()->usableAdapter()->devices();
    qCDebug(BLUEDAEMON) << "List: " << list.length();
    DeviceInfo info;
    Q_FOREACH(Device *const device, list) {
        info[QStringLiteral("name")] = device->friendlyName();
        info[QStringLiteral("icon")] = device->icon();
        info[QStringLiteral("address")] = device->address();
        info[QStringLiteral("UUIDs")] = device->UUIDs().join(QStringLiteral(","));
        devices[device->address()] = info;
    }

    if (!d->m_timer.isActive()) {
        qCDebug(BLUEDAEMON) << "Start Discovery";
        Manager::self()->usableAdapter()->startStableDiscovery();
        d->m_discovered.clear();
        d->m_timer.start();
    }

    Q_FOREACH(const DeviceInfo& info, d->m_discovered) {
        if (!devices.contains(info[QStringLiteral("address")])) {
            devices[info[QStringLiteral("address")]] = info;
        }
    }
    return devices;
}

void BlueDevilDaemon::stopDiscovering()
{
    qCDebug(BLUEDAEMON) << "Stopping discovering";
    d->m_timer.stop();
    Manager::self()->usableAdapter()->stopDiscovery();
}

void BlueDevilDaemon::executeMonolithic()
{
    qCDebug(BLUEDAEMON);

    QProcess process;
    if (!process.startDetached(QStringLiteral("bluedevil-monolithic"))) {
        qCritical() << "Could not start bluedevil-monolithic";
    }
}

void BlueDevilDaemon::killMonolithic()
{
    qCDebug(BLUEDAEMON);
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.bluedevilmonolithic"),
        QStringLiteral("/MainApplication"),
        QStringLiteral("org.kde.KApplication"),
        QStringLiteral("quit")
    );
    QDBusPendingCall pending = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(monolithicQuit(QDBusPendingCallWatcher*)));
}

void BlueDevilDaemon::onlineMode()
{
    qCDebug(BLUEDAEMON);
    if (d->m_status == Private::Online) {
        qCDebug(BLUEDAEMON) << "Already in onlineMode";
        return;
    }

    d->m_bluezAgent = new BluezAgent(new QObject());
    connect(d->m_bluezAgent, SIGNAL(agentReleased()), this, SLOT(agentReleased()));

    connect(d->m_adapter, SIGNAL(deviceFound(Device*)), this, SLOT(deviceFound(Device*)));
    connect(&d->m_timer, SIGNAL(timeout()), d->m_adapter, SLOT(stopDiscovery()));

    FileReceiverSettings::self()->load();
    if (!d->m_fileReceiver && FileReceiverSettings::self()->enabled()) {
        d->m_fileReceiver = new FileReceiver(this);
    }
    if (d->m_fileReceiver && !FileReceiverSettings::self()->enabled()) {
        qCDebug(BLUEDAEMON) << "Stoppping server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    if (!d->m_placesModel) {
        d->m_placesModel = new KFilePlacesModel();
    }

    //Just in case kded was killed or crashed
    QModelIndex index = d->m_placesModel->closestItem(QUrl(QStringLiteral("bluetooth:/")));
    while (index.row() != -1) {
        d->m_placesModel->removePlace(index);
        index = d->m_placesModel->closestItem(QUrl(QStringLiteral("bluetooth:/")));
    }

    d->m_placesModel->addPlace(QStringLiteral("Bluetooth"), QUrl(QStringLiteral("bluetooth:/")), QStringLiteral("preferences-system-bluetooth"));

    executeMonolithic();

    d->m_status = Private::Online;
}

void BlueDevilDaemon::monolithicFinished(const QString &)
{
    qCDebug(BLUEDAEMON);

    if (d->m_status == Private::Online) {
        executeMonolithic();
    }
}

void BlueDevilDaemon::offlineMode()
{
    qCDebug(BLUEDAEMON) << "Offline mode";
    if (d->m_status == Private::Offline) {
        qCDebug(BLUEDAEMON) << "Already in offlineMode";
        return;
    }

    d->m_adapter = 0;

    if (d->m_bluezAgent) {
        delete d->m_bluezAgent->parent(); // we meed to delete the parent for not leaking it
        d->m_bluezAgent = 0;
    }

    if (d->m_fileReceiver) {
        qCDebug(BLUEDAEMON) << "Stoppping server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    //Just to be sure that online was called
    if (d->m_placesModel)  {
        QModelIndex index = d->m_placesModel->closestItem(QUrl(QStringLiteral("bluetooth:/")));
        d->m_placesModel->removePlace(index);
    }

    if (BlueDevil::Manager::self()->adapters().isEmpty()) {
        killMonolithic();
    }
    d->m_status = Private::Offline;
}

/*
 * The agent is released by another agents, for example if an user wants to use
 * blueman agent in kde, we've to respect the user decision here, so ATM until we have
 * the KCM, we should just delete the agent and be quiet
 */
void BlueDevilDaemon::agentReleased()
{
    //TODO think what to do
}

void BlueDevilDaemon::usableAdapterChanged(Adapter *adapter)
{
    //if we have an adapter, remove it and offline the KDED for a moment
    if (d->m_adapter) {
        offlineMode();
    }

    //If the given adapter is not NULL, then set onlineMode again
    if (adapter) {
        d->m_adapter = adapter;
        onlineMode();
    }
}

void BlueDevilDaemon::deviceFound(Device *device)
{
    qCDebug(BLUEDAEMON) << "DeviceFound: " << device->name();
    d->m_discovered.append(deviceToInfo(device));
    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("bluetooth:/")));
}

void BlueDevilDaemon::monolithicQuit(QDBusPendingCallWatcher* watcher)
{
    qCDebug(BLUEDAEMON);
    QDBusPendingReply<void> reply = *watcher;
    if (reply.isError()) {
        qDebug() << "Error response: " << reply.error().message();
        killMonolithic();
    }
}

DeviceInfo BlueDevilDaemon::deviceToInfo(Device *const device) const
{
    DeviceInfo info;
    info[QStringLiteral("name")] = device->friendlyName();
    info[QStringLiteral("icon")] = device->icon();
    info[QStringLiteral("address")] = device->address();
    info[QStringLiteral("discovered")] = QStringLiteral("true");
    info[QStringLiteral("UUIDs")] = device->UUIDs().join(QStringLiteral(","));

    return info;
}

Q_LOGGING_CATEGORY(BLUEDAEMON, "BlueDaemon")

#include "BlueDevilDaemon.moc"
