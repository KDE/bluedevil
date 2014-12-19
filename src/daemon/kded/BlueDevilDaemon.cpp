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

#include <QtCore/QProcess>
#include <QDBusServiceWatcher>
#include <QDBusMetaType>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KComponentData>
#include <KPluginFactory>
#include <kfileplacesmodel.h>
#include <kdirnotify.h>
#include <ksharedconfig.h>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>
#include <bluedevil/bluedevildevice.h>

using namespace BlueDevil;

K_PLUGIN_FACTORY(BlueDevilFactory,
                 registerPlugin<BlueDevilDaemon>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildaemon", "bluedevil"))

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
    QTimer                          m_timer;
    KComponentData                  m_componentData;
    KSharedConfig::Ptr              m_config;
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
    d->m_monolithicWatcher = new QDBusServiceWatcher("org.kde.bluedevilmonolithic"
            , QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this);
    d->m_timer.setSingleShot(true);
    d->m_config = KSharedConfig::openConfig("bluedevilglobalrc");

    KAboutData aboutData(
        "bluedevildaemon",
        "bluedevil",
        ki18n("Bluetooth Daemon"),
        bluedevil_version,
        ki18n("Bluetooth Daemon"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "afiestas@kde.org",
        "http://www.afiestas.org");

    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Maintainer"), "edulix@gmail.com",
        "http://blog.edulix.es");

    aboutData.setProgramIconName("preferences-system-bluetooth");
    d->m_componentData = KComponentData(aboutData);
    connect(d->m_monolithicWatcher, SIGNAL(serviceUnregistered(QString)), SLOT(monolithicFinished(QString)));
    connect(&d->m_timer, SIGNAL(timeout()), SLOT(stopDiscovering()));

    connect(Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)),
            this, SLOT(usableAdapterChanged(Adapter*)));
    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)),
            this, SLOT(adapterAdded(Adapter*)));
    connect(Manager::self(), SIGNAL(adapterRemoved(Adapter*)),
            this, SLOT(adapterRemoved(Adapter*)));

    // Catch suspend/resume events
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         "PrepareForSleep",
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

static Adapter *adapterForAddress(const QString &address)
{
    Q_FOREACH (Adapter *adapter, Manager::self()->adapters()) {
        if (adapter->address() == address) {
            return adapter;
        }
    }
    return 0;
}

void BlueDevilDaemon::login1PrepareForSleep(bool active)
{
    if (active) {
        kDebug(dblue()) << "About to suspend";
        saveAdaptersState();
    } else {
        kDebug(dblue()) << "About to resume";
        restoreAdaptersState();
    }
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

    kDebug(dblue()) << "Start discovering for" << timeout << "ms";

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

    kDebug(dblue()) << "Stop discovering";

    if (d->m_adapter->isDiscovering()) {
        d->m_adapter->stopDiscovery();
    }
}

void BlueDevilDaemon::executeMonolithic()
{
    kDebug(dblue());

    QProcess process;
    if (!process.startDetached("bluedevil-monolithic")) {
        kError() << "Could not start bluedevil-monolithic";
    }
}

void BlueDevilDaemon::killMonolithic()
{
    kDebug(dblue());
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.bluedevilmonolithic",
        "/MainApplication",
        "org.kde.KApplication",
        "quit"
    );
    QDBusPendingCall pending = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(monolithicQuit(QDBusPendingCallWatcher*)));
}

void BlueDevilDaemon::onlineMode()
{
    kDebug(dblue());
    if (d->m_status == Private::Online) {
        kDebug(dblue()) << "Already in onlineMode";
        return;
    }

    d->m_bluezAgent = new BluezAgent(new QObject());
    connect(d->m_bluezAgent, SIGNAL(agentReleased()), this, SLOT(agentReleased()));
    connect(d->m_adapter, SIGNAL(deviceFound(Device*)), this, SLOT(deviceFound(Device*)));

    FileReceiverSettings::self()->readConfig();
    if (!d->m_fileReceiver && FileReceiverSettings::self()->enabled()) {
        d->m_fileReceiver = new FileReceiver(d->m_componentData, this);
    }
    if (d->m_fileReceiver && !FileReceiverSettings::self()->enabled()) {
        kDebug(dblue()) << "Stoppping server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    if (!d->m_placesModel) {
        d->m_placesModel = new KFilePlacesModel();
    }

    //Just in case kded4 was killed or crashed
    QModelIndex index = d->m_placesModel->closestItem(KUrl("bluetooth:/"));
    while (index.row() != -1) {
        d->m_placesModel->removePlace(index);
        index = d->m_placesModel->closestItem(KUrl("bluetooth:/"));
    }

    d->m_placesModel->addPlace("Bluetooth", KUrl("bluetooth:/"), "preferences-system-bluetooth");

    executeMonolithic();

    d->m_status = Private::Online;
}

void BlueDevilDaemon::monolithicFinished(const QString &)
{
    kDebug(dblue());

    if (d->m_status == Private::Online) {
        executeMonolithic();
    }
}

void BlueDevilDaemon::offlineMode()
{
    kDebug(dblue()) << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug(dblue()) << "Already in offlineMode";
        return;
    }

    d->m_adapter = 0;

    if (d->m_bluezAgent) {
        delete d->m_bluezAgent->parent(); // we meed to delete the parent for not leaking it
        d->m_bluezAgent = 0;
    }

    if (d->m_fileReceiver) {
        kDebug(dblue()) << "Stoppping server";
        delete d->m_fileReceiver;
        d->m_fileReceiver = 0;
    }

    //Just to be sure that online was called
    if (d->m_placesModel)  {
        QModelIndex index = d->m_placesModel->closestItem(KUrl("bluetooth:/"));
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
    kDebug(dblue()) << "DeviceFound: " << device->name();
    org::kde::KDirNotify::emitFilesAdded("bluetooth:/");
}

void BlueDevilDaemon::monolithicQuit(QDBusPendingCallWatcher* watcher)
{
    kDebug(dblue());
    QDBusPendingReply<void> reply = *watcher;
    if (reply.isError()) {
        qDebug() << "Error response: " << reply.error().message();
        killMonolithic();
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
        const QString key = QString("%1_powered").arg(adapter->address());
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
        const QString key = QString("%1_powered").arg(adapter->address());
        adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
    }
}

void BlueDevilDaemon::restoreAdapterState(Adapter *adapter)
{
    KConfigGroup adaptersGroup = d->m_config->group("Adapters");

    const QString key = QString("%1_powered").arg(adapter->address());
    adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
}

DeviceInfo BlueDevilDaemon::deviceToInfo(Device *const device) const
{
    DeviceInfo info;
    info["name"] = device->friendlyName();
    info["icon"] = device->icon();
    info["address"] = device->address();
    info["UBI"] = device->UBI();
    info["UUIDs"] = device->UUIDs().join(",");
    return info;
}

extern int dblue() { static int s_area = KDebug::registerArea("BlueDaemon", false); return s_area; }
