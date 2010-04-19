/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "BlueDevilDaemon.h"

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>
#include <solid/control/bluetoothmanager.h>
#include "agentlistener.h"
#include "bluedevil_service_interface.h"
#include <solid/control/bluetoothinterface.h>

K_PLUGIN_FACTORY(BlueDevilFactory,
                 registerPlugin<BlueDevilDaemon>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildaemon", "bluedevil"))

struct BlueDevilDaemon::Private
{
    enum Status {Online = 0, Offline} status;
    //Do not delete this :)
    Solid::Control::BluetoothManager* man;
    AgentListener *agentListener;
    Solid::Control::BluetoothInterface* adapter;
    org::kde::BlueDevil::Service* service;
    QString m_defaultAdapterName;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent), d(new Private)
{
    d->agentListener = 0;
    d->adapter = 0;
    d->service = 0;

    KGlobal::locale()->insertCatalog("bluedevil");

    KAboutData aboutData(
        "BlueDevil",
        "bluedevil",
        ki18n("BlueDevil"),
        "1.0",
        ki18n("KDE Bluetooth System"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, Artesanos del Software")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "alex@eyeos.org",
        "http://www.afiestas.org");

    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Maintainer"), "edulix@gmail.com",
        "http://blog.edulix.es");

    //Status = offline ATM
    d->status = Private::Offline;

    d->man = &Solid::Control::BluetoothManager::self();

    connect(d->man,SIGNAL(interfaceAdded(const QString&)),this,SLOT(adapterAdded(const QString&)));
    connect(d->man,SIGNAL(interfaceRemoved(const QString&)),this,SLOT(adapterRemoved(const QString&)));
    connect(d->man,SIGNAL(defaultInterfaceChanged(const QString&)),this,SLOT(defaultAdapterChanged(const QString&)));

    if ( d->man->bluetoothInterfaces().size() > 0 ) {
        onlineMode();
    }
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    delete d;
}

bool BlueDevilDaemon::serviceStarted()
{
    d->service = new org::kde::BlueDevil::Service("org.kde.BlueDevil.Service",
        "/Service", QDBusConnection::sessionBus(), this);

    if ((QString)d->service->ping() == "pong") {
        kDebug() << "org::kde::BlueDevil::Service is up and running!";
        return true;
    } else {
        kDebug() << d->service->ping();
        return false;
    }
}

void BlueDevilDaemon::onlineMode()
{
    kDebug();
    if (d->status == Private::Online) {
        kDebug() << "Already in onlineMode";
        return;
    }

    d->agentListener = new AgentListener();
    connect(d->agentListener,SIGNAL(agentReleased()),this,SLOT(agentReleased()));
    d->agentListener->start();

    d->adapter = new Solid::Control::BluetoothInterface(d->man->defaultInterface());
    if (!serviceStarted()) {
      return;
    }
    d->service->launchServer();

    d->status = Private::Online;
}

void BlueDevilDaemon::offlineMode()
{
    if (d->status == Private::Offline) {
        kDebug() << "Already in offlineMode";
        return;
    }
    kDebug() << "Offline mode";

    connect(d->agentListener,SIGNAL(finished()),this,SLOT(agentThreadStopped()));
    d->agentListener->quit();

    kDebug() << "You've got no bluetooth interfaces attached!";
    d->status = Private::Offline;

    if (!serviceStarted()) {
      return;
    }
    d->service->stopServer();
}

/*
 * The agent is released by another agents, for example if an user wants to use
 * blueman agent in kde, we've to respect the user decision here, so ATM until we have
 * the KCM, we should just delete the agent and be quiet
 */
void BlueDevilDaemon::agentReleased()
{
    //If this code grows just one line, put it in a method
    connect(d->agentListener,SIGNAL(finished()),this,SLOT(agentThreadStopped()));
    d->agentListener->quit();
}

void BlueDevilDaemon::agentThreadStopped()
{
    d->agentListener->deleteLater();
    d->agentListener = 0;

    kDebug() << "agent listener deleted";
}

void BlueDevilDaemon::adapterAdded(const QString& adapterName)
{
    kDebug() << adapterName;
    if (!d->man->bluetoothInterfaces().isEmpty() && d->status == Private::Offline) {
        d->m_defaultAdapterName = adapterName;
        onlineMode();
    }
}

void BlueDevilDaemon::adapterRemoved(const QString& adapterName)
{
    kDebug() << adapterName;
    if (d->man->bluetoothInterfaces().isEmpty()) {
        d->m_defaultAdapterName = QString();
        offlineMode();
    }
}

void BlueDevilDaemon::defaultAdapterChanged(const QString& adapterName)
{
    kDebug() << adapterName;
    //This should do the trick :)
    if (d->m_defaultAdapterName == adapterName && d->status == Private::Online) {
      kDebug() << "already online with that adapter";
      return;
    }
    offlineMode();
    onlineMode();
}
