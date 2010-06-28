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
#include "openobex/serversession.h"

#include <KDebug>
#include <bluedevil/bluedevilmanager.h>
#include <bluedevil/bluedeviladapter.h>

#include <QtCore/QCoreApplication>

Service::Service()
{
    new ServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.kde.BlueDevil.Service");
    dbus.registerObject("/Service", this);
    m_server = 0;
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
    if (m_server) {
        return;
    }
    m_server = new OpenObex::Server(BlueDevil::Manager::self()->defaultAdapter()->address());
    kDebug() << m_server;
}

void Service::stopServer()
{
    kDebug() << m_server;
    
    if (!m_server) {
        return;
    }

    m_server->deleteLater();
    m_server = 0;

    // After 10 seconds, if server is not restarted, terminate the helper
    QTimer::singleShot(10000, this, SLOT(quit()));
}

void Service::quit()
{
    kDebug();
    // Only quit if no server is running
    if (!m_server) {
        deleteLater();
    }
}
