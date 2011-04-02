/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "service.h"
#include "serviceadaptor.h"
#include "filereceiversettings.h"
#include "openobex/serverftp.h"
#include "openobex/serversession.h"

#include <KDebug>
#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

#include <QDBusServiceWatcher>
#include <QtCore/QCoreApplication>

Service::Service()
{
    new ServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.kde.BlueDevil.Service");
    dbus.registerObject("/Service", this);
    m_server = 0;
    m_serverftp = 0;
    m_watcher = 0;
}

Service::~Service()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject("/Service");
    dbus.unregisterService("org.kde.BlueDevil.Service");
    qApp->quit();
}

void Service::launchServer()
{
    if (m_server && m_serverftp) {
        return;
    }

    if (!m_watcher) {
        m_watcher = new QDBusServiceWatcher("org.openobex", QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration);
        connect(m_watcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(openobexUnregistered()));
    }

    if (BlueDevil::Manager::self()->defaultAdapter()) {

        FileReceiverSettings::self()->readConfig();
        if (FileReceiverSettings::enabled()) {
            if (!m_server) {
                kDebug() << "Launching Server";
                m_server = new OpenObex::Server(BlueDevil::Manager::self()->defaultAdapter()->address());
            }
        }
        if (FileReceiverSettings::shareEnabled()) {
            if (!m_serverftp) {
                kDebug() << "Launching FileSharing";
                m_serverftp = new OpenObex::ServerFtp(BlueDevil::Manager::self()->defaultAdapter()->address());
            }
        }
    } else{
        kDebug() << "No adapters found";
    }
}

void Service::stopServer()
{
    kDebug() << m_server;
    kDebug() << m_serverftp;

    if (!m_server && !m_serverftp) {
        return;
    }

    delete m_server;
    m_server = 0;

    delete m_serverftp;
    m_serverftp = 0;
}

bool Service::isRunning()
{
    if (!m_server) {
        return false;
    }

    return true;
}

void Service::openobexUnregistered()
{
    kDebug();
    stopServer();

    //Try to re-execute obex-data-server since it probably has crashed
    launchServer();
}