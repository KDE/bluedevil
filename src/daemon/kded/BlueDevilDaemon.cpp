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
#include "debug_p.h"

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

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>

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

    QBluez::Manager *m_manager;
    QBluez::Adapter *m_adapter;
    BluezAgent *m_bluezAgent;
    KFilePlacesModel *m_placesModel;
    QDBusServiceWatcher *m_monolithicWatcher;
    FileReceiver *m_fileReceiver;
    QTimer m_timer;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    d->m_manager = new QBluez::Manager(this);
    d->m_bluezAgent = 0;
    d->m_adapter = 0;
    d->m_placesModel = 0;
    d->m_fileReceiver = 0;
    d->m_monolithicWatcher = new QDBusServiceWatcher(QStringLiteral("org.kde.bluedevilmonolithic"),
            QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this);
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

    d->m_status = Private::Offline;

    // Initialize QBluez
    QBluez::InitManagerJob *initJob = d->m_manager->init();
    initJob->start();
    connect(initJob, &QBluez::InitManagerJob::result, this, &BlueDevilDaemon::initJobResult);
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

QMapDeviceInfo BlueDevilDaemon::allDevices()
{
    QMapDeviceInfo devices;
    const QList<QBluez::Device*> &list = d->m_adapter->devices();

    Q_FOREACH(QBluez::Device *const device, list) {
        DeviceInfo info;
        info[QStringLiteral("name")] = device->friendlyName();
        info[QStringLiteral("icon")] = device->icon();
        info[QStringLiteral("address")] = device->address();
        info[QStringLiteral("ubi")] = device->ubi();
        info[QStringLiteral("UUIDs")] = device->uuids().join(QStringLiteral(","));
        devices[device->address()] = info;
    }

    return devices;
}

DeviceInfo BlueDevilDaemon::device(const QString &address)
{
    QBluez::Device *device = d->m_manager->deviceForAddress(address);
    if (!device) {
        return DeviceInfo();
    }

    DeviceInfo info;
    info[QStringLiteral("name")] = device->friendlyName();
    info[QStringLiteral("icon")] = device->icon();
    info[QStringLiteral("address")] = device->address();
    info[QStringLiteral("UUIDs")] = device->uuids().join(QStringLiteral(","));
    return info;
}

void BlueDevilDaemon::startDiscovering(quint32 timeout)
{
    qCDebug(BLUEDAEMON) << "Starting discovering for" << timeout << "seconds";

    d->m_adapter->startDiscovery();

    if (timeout > 0) {
        d->m_timer.start(timeout * 1000);
    }
}

void BlueDevilDaemon::stopDiscovering()
{
    qCDebug(BLUEDAEMON) << "Stopping discovering";

    if (d->m_adapter->isDiscovering()) {
        d->m_adapter->stopDiscovery();
    }
}

void BlueDevilDaemon::executeMonolithic()
{
    qCDebug(BLUEDAEMON) << "Execute monolithic";

    QProcess process;
    if (!process.startDetached(QStringLiteral("bluedevil-monolithic"))) {
        qCCritical(BLUEDAEMON) << "Could not start bluedevil-monolithic";
    }
}

void BlueDevilDaemon::killMonolithic()
{
    qCDebug(BLUEDAEMON) << "Kill monolithic";

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
    qCDebug(BLUEDAEMON) << "Going to online mode";

    if (d->m_status == Private::Online) {
        qCDebug(BLUEDAEMON) << "Already in onlineMode";
        return;
    }

    d->m_bluezAgent = new BluezAgent(this);
    d->m_manager->registerAgent(d->m_bluezAgent);
    d->m_manager->requestDefaultAgent(d->m_bluezAgent);
    connect(d->m_bluezAgent, SIGNAL(agentReleased()), this, SLOT(agentReleased()));

    connect(d->m_adapter, &QBluez::Adapter::deviceFound, this, &BlueDevilDaemon::deviceFound);
    connect(&d->m_timer, &QTimer::timeout, d->m_adapter, &QBluez::Adapter::stopDiscovery);

    FileReceiverSettings::self()->load();
    if (!d->m_fileReceiver && FileReceiverSettings::self()->enabled()) {
        d->m_fileReceiver = new FileReceiver(this);
    }
    if (d->m_fileReceiver && !FileReceiverSettings::self()->enabled()) {
        qCDebug(BLUEDAEMON) << "Stoppping file receiver server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    if (!d->m_placesModel) {
        d->m_placesModel = new KFilePlacesModel();
    }

    // Just in case kded was killed or crashed
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
    qCDebug(BLUEDAEMON) << "Going to offline mode";

    if (d->m_status == Private::Offline) {
        qCDebug(BLUEDAEMON) << "Already in offlineMode";
        return;
    }

    d->m_adapter = 0;

    if (d->m_bluezAgent) {
        d->m_manager->unregisterAgent(d->m_bluezAgent);
        delete d->m_bluezAgent;
        d->m_bluezAgent = 0;
    }

    if (d->m_fileReceiver) {
        qCDebug(BLUEDAEMON) << "Stoppping file receiver server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    // Just to be sure that online was called
    if (d->m_placesModel)  {
        QModelIndex index = d->m_placesModel->closestItem(QUrl(QStringLiteral("bluetooth:/")));
        d->m_placesModel->removePlace(index);
    }

    if (d->m_manager->adapters().isEmpty()) {
        killMonolithic();
    }
    d->m_status = Private::Offline;
}

void BlueDevilDaemon::initJobResult(QBluez::InitManagerJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDAEMON) << "Error initializing manager:" << job->errorText();
        return;
    }

    usableAdapterChanged(d->m_manager->usableAdapter());
    connect(d->m_manager, &QBluez::Manager::usableAdapterChanged, this, &BlueDevilDaemon::usableAdapterChanged);

    Q_FOREACH (QBluez::Device *device, d->m_manager->devices()) {
        deviceFound(device);
    }

    if (!d->m_manager->adapters().isEmpty()) {
        executeMonolithic();
    }
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

void BlueDevilDaemon::usableAdapterChanged(QBluez::Adapter *adapter)
{
    // If we have an adapter, remove it and offline the KDED for a moment
    if (d->m_adapter) {
        offlineMode();
    }

    // If the given adapter is not NULL, then set onlineMode again
    if (adapter) {
        d->m_adapter = adapter;
        onlineMode();
    }
}

void BlueDevilDaemon::deviceFound(QBluez::Device *device)
{
    qCDebug(BLUEDAEMON) << "DeviceFound: " << device->ubi();

    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("bluetooth:/")));
}

void BlueDevilDaemon::monolithicQuit(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<void> reply = *watcher;
    if (reply.isError()) {
        qCDebug(BLUEDAEMON) << "Error response: " << reply.error().message();
        killMonolithic();
    }
}

DeviceInfo BlueDevilDaemon::deviceToInfo(QBluez::Device *const device) const
{
    DeviceInfo info;
    info[QStringLiteral("name")] = device->friendlyName();
    info[QStringLiteral("icon")] = device->icon();
    info[QStringLiteral("address")] = device->address();
    info[QStringLiteral("discovered")] = QStringLiteral("true");
    info[QStringLiteral("UUIDs")] = device->uuids().join(QStringLiteral(","));

    return info;
}

#include "BlueDevilDaemon.moc"
