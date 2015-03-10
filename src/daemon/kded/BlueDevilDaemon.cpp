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
#include <ksharedconfig.h>

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

    BluezAgent *m_bluezAgent;
    KFilePlacesModel *m_placesModel;
    Adapter *m_adapter;
    QDBusServiceWatcher *m_monolithicWatcher;
    FileReceiver *m_fileReceiver;
    KSharedConfig::Ptr m_config;
    QTimer m_timer;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    d->m_bluezAgent = 0;
    d->m_adapter = 0;
    d->m_placesModel = 0;
    d->m_fileReceiver = 0;
    d->m_monolithicWatcher = new QDBusServiceWatcher(QStringLiteral("org.kde.bluedevilmonolithic")
            , QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this);
    d->m_timer.setSingleShot(true);
    d->m_config = KSharedConfig::openConfig(QStringLiteral("bluedevilglobalrc"));

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

    aboutData.setComponentName(QStringLiteral("bluedevil"));
    KAboutData::registerPluginData(aboutData);

    connect(d->m_monolithicWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &BlueDevilDaemon::monolithicFinished);
    connect(&d->m_timer, &QTimer::timeout, this, &BlueDevilDaemon::stopDiscovering);

    connect(Manager::self(), &Manager::usableAdapterChanged, this, &BlueDevilDaemon::usableAdapterChanged);
    connect(Manager::self(), &Manager::adapterAdded, this, &BlueDevilDaemon::adapterAdded);
    connect(Manager::self(), &Manager::adapterRemoved, this, &BlueDevilDaemon::adapterRemoved);

    // Catch suspend/resume events
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(login1PrepareForSleep(bool))
                                         );

    d->m_status = Private::Offline;

    restoreAdaptersState();
    usableAdapterChanged(Manager::self()->usableAdapter());

    if (!Manager::self()->adapters().isEmpty()) {
        executeMonolithic();
    }
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    saveAdaptersState();

    if (d->m_status == Private::Online) {
        offlineMode();
    }

    delete d;
}

void BlueDevilDaemon::login1PrepareForSleep(bool active)
{
    if (active) {
        qCDebug(BLUEDAEMON) << "About to suspend";
        saveAdaptersState();
    } else {
        qCDebug(BLUEDAEMON) << "About to resume";
        restoreAdaptersState();
    }
}

bool BlueDevilDaemon::isOnline()
{
    return d->m_status == Private::Online;
}

QMapDeviceInfo BlueDevilDaemon::allDevices()
{
    QMapDeviceInfo devices;
    QList<Device*> list = Manager::self()->usableAdapter()->devices();

    Q_FOREACH (Device *const device, list) {
        devices[device->address()] = deviceToInfo(device);
    }

    return devices;
}

DeviceInfo BlueDevilDaemon::device(const QString &address)
{
    Q_FOREACH (Device *const device, Manager::self()->devices()) {
        if (device->address() == address) {
            return deviceToInfo(device);
        }
    }

    return DeviceInfo();
}

void BlueDevilDaemon::startDiscovering(quint32 timeout)
{
    if (!d->m_adapter) {
        return;
    }

    qCDebug(BLUEDAEMON) << "Start discovering for" << timeout << "ms";

    d->m_adapter->startDiscovery();

    if (timeout > 0) {
        d->m_timer.start(timeout);
    }
}

void BlueDevilDaemon::stopDiscovering()
{
    if (!d->m_adapter) {
        return;
    }

    qCDebug(BLUEDAEMON) << "Stop discovering";

    if (d->m_adapter->isDiscovering()) {
        d->m_adapter->stopDiscovery();
    }
}

void BlueDevilDaemon::executeMonolithic()
{
    QProcess process;
    if (!process.startDetached(QStringLiteral("bluedevil-monolithic"))) {
        qCCritical(BLUEDAEMON) << "Could not start bluedevil-monolithic";
    }
}

void BlueDevilDaemon::killMonolithic()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.bluedevilmonolithic"),
        QStringLiteral("/MainApplication"),
        QStringLiteral("org.kde.KApplication"),
        QStringLiteral("quit")
    );

    QDBusPendingCall pending = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &BlueDevilDaemon::monolithicQuit);
}

void BlueDevilDaemon::onlineMode()
{
    if (d->m_status == Private::Online) {
        qCDebug(BLUEDAEMON) << "Already in onlineMode";
        return;
    }

    d->m_bluezAgent = new BluezAgent(new QObject());
    connect(d->m_bluezAgent, &BluezAgent::agentReleased, this, &BlueDevilDaemon::agentReleased);
    connect(d->m_adapter, &Adapter::deviceFound, this, &BlueDevilDaemon::deviceFound);

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
        delete d->m_bluezAgent->parent(); // we need to delete the parent for not leaking it
        d->m_bluezAgent = 0;
    }

    if (d->m_fileReceiver) {
        qCDebug(BLUEDAEMON) << "Stoppping server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    // Just to be sure that online was called
    if (d->m_placesModel)  {
        QModelIndex index = d->m_placesModel->closestItem(QUrl(QStringLiteral("bluetooth:/")));
        d->m_placesModel->removePlace(index);
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

void BlueDevilDaemon::adapterAdded(Adapter *adapter)
{
    restoreAdapterState(adapter);
}

void BlueDevilDaemon::adapterRemoved(Adapter *adapter)
{
    Q_UNUSED(adapter)

    if (BlueDevil::Manager::self()->adapters().isEmpty()) {
        killMonolithic();
    }
}

void BlueDevilDaemon::deviceFound(Device *device)
{
    qCDebug(BLUEDAEMON) << "DeviceFound: " << device->name();
    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("bluetooth:/")));
}

void BlueDevilDaemon::monolithicQuit(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<void> reply = *watcher;
    if (reply.isError()) {
        qCDebug(BLUEDAEMON) << "Error response: " << reply.error().message();
    }
}

void BlueDevilDaemon::saveAdaptersState()
{
    Manager *manager = Manager::self();
    if (!manager) {
        return;
    }

    KConfigGroup adaptersGroup = d->m_config->group("Adapters");

    Q_FOREACH (Adapter *adapter, manager->adapters()) {
        const QString key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
        adaptersGroup.writeEntry<bool>(key, adapter->isPowered());
    }

    d->m_config->sync();
}

// New adapters are automatically powered on
void BlueDevilDaemon::restoreAdaptersState()
{
    Manager *manager = Manager::self();
    if (!manager) {
        return;
    }

    KConfigGroup adaptersGroup = d->m_config->group("Adapters");

    Q_FOREACH (Adapter *adapter, manager->adapters()) {
        const QString key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
        adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
    }
}

void BlueDevilDaemon::restoreAdapterState(Adapter *adapter)
{
    KConfigGroup adaptersGroup = d->m_config->group("Adapters");

    const QString key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
    adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
}

DeviceInfo BlueDevilDaemon::deviceToInfo(Device *const device) const
{
    DeviceInfo info;

    info[QStringLiteral("name")] = device->friendlyName();
    info[QStringLiteral("icon")] = device->icon();
    info[QStringLiteral("address")] = device->address();
    info[QStringLiteral("UBI")] = device->UBI();
    info[QStringLiteral("UUIDs")] = device->UUIDs().join(QLatin1Char(','));

    return info;
}

Q_LOGGING_CATEGORY(BLUEDAEMON, "BlueDaemon")

#include "BlueDevilDaemon.moc"
