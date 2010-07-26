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
#include "filereceiversettings.h"

#include <kdemacros.h>
#include <KDebug>
#include <KAboutData>
#include <KPluginFactory>
#include <kfileplacesmodel.h>
#include <kprocess.h>

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

    AgentListener                   *m_agentListener;
    KFilePlacesModel                *m_placesModel;
    BlueDevil::Adapter              *m_adapter;
    org::kde::BlueDevil::Service    *m_service;
};

BlueDevilDaemon::BlueDevilDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , d(new Private)
{
    d->m_agentListener = 0;
    d->m_adapter = 0;
    d->m_service = 0;
    d->m_placesModel = 0;

    KGlobal::locale()->insertCatalog("bluedevil");

    KAboutData aboutData(
        "bluedevil_daemon",
        "bluedevil",
        ki18n("BlueDevil Daemon"),
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

    KProcess process;
    process.startDetached("bluedevil-monolithic");
}

BlueDevilDaemon::~BlueDevilDaemon()
{
    if (d->m_status == Private::Online) {
        offlineMode();
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.bluedevil-monolithic",
        "/MainApplication",
        "org.kde.KApplication",
        "quit"
    );
    QDBusConnection::sessionBus().asyncCall(msg);
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

    FileReceiverSettings::self()->readConfig();
    if (!isServiceStarted() && FileReceiverSettings::self()->enabled()) {
        kDebug() << "Launching srever";
        d->m_service->launchServer();
    }

    if (!d->m_placesModel) {
        d->m_placesModel = new KFilePlacesModel();
    }
    d->m_placesModel->addPlace("Bluetooth", KUrl("bluetooth:/"), "preferences-system-bluetooth");
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
