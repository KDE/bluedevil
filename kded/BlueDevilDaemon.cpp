/***************************************************************************
 *   Copyright (C) 2010 by Alex Fiestas <alex@eyeos.org>                   *
 *   Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************
 *                                                                         *
 *   Part of the code in this file was taken from KDE4Bluesave and/or     *
 *   Lithium, where noticed.                                               *
 *                                                                         *
 **************************************************************************/

#include "BlueDevilDaemon.h"

#define BLUEDEVIL_VERSION "0.1"
#include <kdemacros.h>
#include <kdebug.h>
#include <KAboutData>
#include <KPluginFactory>
#include <QWidget>
#include <solid/control/bluetoothmanager.h>
#include "agentlistener.h"
#include <solid/control/bluetoothinterface.h>
#include "openobex/server.h"

K_PLUGIN_FACTORY(BlueDevilFactory,
                 registerPlugin<BlueDevilDaemon>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildaemon"))

struct BlueDevilDaemon::Private
{
    bool status;
    //Do not delete this :)
    Solid::Control::BluetoothManager* man;
    AgentListener *agentListener;
    Solid::Control::BluetoothInterface* adapter;
    OpenObex::Server *server;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent),
    d(new Private)
{
    d->agentListener = 0;
    d->adapter = 0;
    d->server = 0;

    KGlobal::locale()->insertCatalog("bluedevil");

    KAboutData aboutData("bluedevil", "bluedevil", ki18n("BlueDevil"),
        BLUEDEVIL_VERSION, ki18n("A Bluetooth Management tool for KDE4"),
        KAboutData::License_GPL, ki18n("(c) 2010 Artesanos del Software "),
        KLocalizedString(), "http://www.kde.org");

    aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Maintainer"), "alex@eyeos.org",
        "http://www.afiestas.org");

    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Maintainer"), "edulix@gmail.com",
        "http://blog.edulix.es");

    //Status = offline ATM
    d->status = false;

    d->man = &Solid::Control::BluetoothManager::self();

    connect(d->man,SIGNAL(interfaceAdded(const QString&)),this,SLOT(adapterAdded(const QString&)));
    connect(d->man,SIGNAL(interfaceRemoved(const QString&)),this,SLOT(adapterRemoved(const QString&)));
    connect(d->man,SIGNAL(defaultInterfaceChanged(const QString&)),this,SLOT(defaultAdapterChanged(const QString&)));

    if ( d->man->bluetoothInterfaces().size() > 0 ) {
        onlineMode();
    } else {
        offlineMode();
    }
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    delete d;
}

void BlueDevilDaemon::onlineMode()
{
    if (d->status == true) {
        qDebug() << "Already in onlineMode";
        return;
    }

    qDebug() << "Online mode";

    d->agentListener = new AgentListener();
    connect(d->agentListener,SIGNAL(agentReleased()),this,SLOT(agentReleased()));
    d->agentListener->start();

    d->adapter = new Solid::Control::BluetoothInterface(d->man->defaultInterface());
    d->server = new OpenObex::Server(d->adapter->address());

    d->status = true;
}

void BlueDevilDaemon::offlineMode()
{
    if (d->status == false) {
        qDebug() << "Already in offlineMode";
        return;
    }
    qDebug() << "Offline mode";

    connect(d->agentListener,SIGNAL(finished()),this,SLOT(agentThreadStopped()));
    d->agentListener->quit();

    qDebug() << "You've got no bluetooth interfaces attached!";
    d->status = false;

    if (d->server) {
        d->server->close();
        connect(d->server, SIGNAL(closed()), this, SLOT(serverClosed()));
    }
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

    qDebug() << "agent listener deleted";
}

void BlueDevilDaemon::serverClosed()
{
    delete d->server;
    d->server = 0;
}

void BlueDevilDaemon::adapterAdded(const QString& adapterName)
{
    qDebug() << adapterName;
    if (d->man->bluetoothInterfaces().size() > 0 && d->status == false) {
        onlineMode();
    }
}

void BlueDevilDaemon::adapterRemoved(const QString& adapterName)
{
    qDebug() << adapterName;
    if (d->man->bluetoothInterfaces().size() < 1) {
        offlineMode();
    }
}

void BlueDevilDaemon::defaultAdapterChanged(const QString& adapterName)
{
    qDebug() << adapterName;
}
