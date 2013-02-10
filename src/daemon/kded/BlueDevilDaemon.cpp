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
#include "bluedevil_service_interface.h"
#include "filereceiversettings.h"
#include "version.h"

#include <QDBusMetaType>

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>
#include <kfileplacesmodel.h>
#include <kprocess.h>
#include <kdirnotify.h>

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
    org::kde::BlueDevil::Service    *m_service;
    QList <DeviceInfo>                m_discovered;
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
    d->m_service = 0;
    d->m_placesModel = 0;
    d->m_timer.setInterval(20000);
    d->m_timer.setSingleShot(true);

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

    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)),
            this, SLOT(defaultAdapterChanged(Adapter*)));

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this, SLOT(deviceFound(Device*)));
    connect(&d->m_timer, SIGNAL(timeout()), Manager::self()->defaultAdapter(), SLOT(stopDiscovery()));

    d->m_status = Private::Offline;
    if (Manager::self()->defaultAdapter()) {
        onlineMode();
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

    QList <Device* > list = Manager::self()->defaultAdapter()->devices();
    kDebug() << "List: " << list.length();
    DeviceInfo info;
    Q_FOREACH(const Device* device, list) {
        info["name"] = device->friendlyName();
        info["icon"] = device->icon();
        info["address"] = device->address();
        devices[device->address()] = info;
    }

    if (!d->m_timer.isActive()) {
        kDebug() << "Start Discovery";
        Manager::self()->defaultAdapter()->startStableDiscovery();
        d->m_discovered.clear();
        d->m_timer.start();
    }

    Q_FOREACH(const DeviceInfo& info, d->m_discovered) {
        if (!devices.contains(info["address"])) {
            devices[info["address"]] = info;
        }
    }
    return devices;
}

void BlueDevilDaemon::stopDiscovering()
{
    kDebug() << "Stopping discovering";
    d->m_timer.stop();
    Manager::self()->defaultAdapter()->stopDiscovery();
}

bool BlueDevilDaemon::isServiceStarted()
{
    if (!d->m_service) {
        d->m_service = new org::kde::BlueDevil::Service("org.kde.BlueDevil.Service",
            "/Service", QDBusConnection::sessionBus(), this);
    }
    QDBusPendingReply <bool > r = d->m_service->isRunning();
    r.waitForFinished();
    if (r.isError() || !r.isValid()) {
        return false;
    }
    return r.value();
}

void BlueDevilDaemon::onlineMode()
{
    kDebug();
    if (d->m_status == Private::Online) {
        kDebug() << "Already in onlineMode";
        return;
    }

    d->m_bluezAgent = new BluezAgent(new QObject());
    connect(d->m_bluezAgent, SIGNAL(agentReleased()), this, SLOT(agentReleased()));

    d->m_adapter = Manager::self()->defaultAdapter();

    FileReceiverSettings::self()->readConfig();
    if (!isServiceStarted() && FileReceiverSettings::self()->enabled()) {
        kDebug() << "Launching server";
        d->m_service->launchServer();
    }
    if (isServiceStarted() && !FileReceiverSettings::self()->enabled()) {
        kDebug() << "Stoppping server";
        d->m_service->stopServer();
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

    KProcess process;
    process.startDetached("bluedevil-monolithic");

    d->m_status = Private::Online;
}

void BlueDevilDaemon::offlineMode()
{
    kDebug() << "Offline mode";
    if (d->m_status == Private::Offline) {
        kDebug() << "Already in offlineMode";
        return;
    }

    d->m_adapter = 0;

    if (d->m_bluezAgent) {
        delete d->m_bluezAgent->parent(); // we meed to delete the parent for not leaking it
        d->m_bluezAgent = 0;
    }

    if (isServiceStarted()) {
        kDebug() << "Stoppping server";
        d->m_service->stopServer();
    }

    //Just to be sure that online was called
    if (d->m_placesModel)  {
        QModelIndex index = d->m_placesModel->closestItem(KUrl("bluetooth:/"));
        d->m_placesModel->removePlace(index);
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.bluedevilmonolithic",
        "/MainApplication",
        "org.kde.KApplication",
        "quit"
    );
    QDBusConnection::sessionBus().asyncCall(msg);
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

void BlueDevilDaemon::defaultAdapterChanged(Adapter *adapter)
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
    kDebug() << "DeviceFound: " << device->name();
    d->m_discovered.append(deviceToInfo(device));
    org::kde::KDirNotify::emitFilesAdded("bluetooth:/");
}

DeviceInfo BlueDevilDaemon::deviceToInfo(const Device* device) const
{
    DeviceInfo info;
    info["name"] = device->friendlyName();
    info["icon"] = device->icon();
    info["address"] = device->address();
    info["discovered"] = "true";

    return info;
}