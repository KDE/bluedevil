/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "BlueDevilDaemon.h"
#include "agentlistener.h"
#include "bluedevil_service_interface.h"

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>
#include <QWidget>

#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

K_PLUGIN_FACTORY(BlueDevilFactory,
                 registerPlugin<BlueDevilDaemon>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildaemon", "bluedevil"))

struct BlueDevilDaemon::Private
{
    enum Status {
        Online = 0,
        Offline
    } m_status;

    AgentListener                *m_agentListener;
    BlueDevil::Adapter           *m_adapter;
    org::kde::BlueDevil::Service *m_service;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    d->m_agentListener = 0;
    d->m_adapter = 0;
    d->m_service = 0;

    KGlobal::locale()->insertCatalog("bluedevil");

    KAboutData aboutData(
        "BlueDevil",
        "bluedevil",
        ki18n("BlueDevil"),
        "1.0",
        ki18n("KDE Bluetooth System"),
        KAboutData::License_GPL,
        ki18n("(c) 2010, UFO Coders")
    );

    aboutData.addAuthor(ki18n("Alejandro Fiestas Olivares"), ki18n("Maintainer"), "alex@eyeos.org",
        "http://www.afiestas.org");

    aboutData.addAuthor(ki18n("Eduardo Robles Elvira"), ki18n("Maintainer"), "edulix@gmail.com",
        "http://blog.edulix.es");

    connect(BlueDevil::Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)),
            this, SLOT(defaultAdapterChanged(Adapter*)));

    d->m_status = Private::Offline;
    if (BlueDevil::Manager::self()->defaultAdapter()) {
        onlineMode();
    }
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    offlineMode();
    delete d;
}

bool BlueDevilDaemon::isServiceStarted()
{
    d->m_service = new org::kde::BlueDevil::Service("org.kde.BlueDevil.Service",
        "/Service", QDBusConnection::sessionBus(), this);
    return d->m_service->isValid();
}

void BlueDevilDaemon::onlineMode()
{
    kDebug();
    if (d->m_status == Private::Online) {
        kDebug() << "Already in onlineMode";
        return;
    }

    d->m_agentListener = new AgentListener();
    connect(d->m_agentListener, SIGNAL(agentReleased()), this, SLOT(agentReleased()));
    d->m_agentListener->start();

    d->m_adapter = BlueDevil::Manager::self()->defaultAdapter();
    if (!isServiceStarted()) {
        kDebug() << "Launching srever";
        d->m_service->launchServer();
    }

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

    connect(d->m_agentListener, SIGNAL(finished()), this, SLOT(agentThreadStopped()));
    d->m_agentListener->quit();

    if (isServiceStarted()) {
        d->m_service->stopServer();
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
    connect(d->m_agentListener,SIGNAL(finished()),this,SLOT(agentThreadStopped()));
    d->m_agentListener->quit();
}

void BlueDevilDaemon::agentThreadStopped()
{
    d->m_agentListener->deleteLater();
    d->m_agentListener = 0;

    kDebug() << "agent listener deleted";
}

void BlueDevilDaemon::defaultAdapterChanged(BlueDevil::Adapter *adapter)
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
