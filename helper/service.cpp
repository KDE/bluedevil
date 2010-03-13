/***************************************************************************
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include <KDebug>

Service::Service()
{
    kDebug(4567);
    new ServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Service", this);
    dbus.registerService("org.kde.BlueDevil.Service");
    m_server = 0;
}

Service::~Service()
{
    delete m_server;
}

void Service::launchServer()
{
    if (m_server) {
        return;
    }

    Solid::Control::BluetoothInterface *adapter = new Solid::Control::BluetoothInterface(
        Solid::Control::BluetoothManager::self().defaultInterface());
    m_server = new OpenObex::Server(adapter->address());
}

void Service::stopServer()
{
  m_server->deleteLater();
}

QString Service::ping()
{
    return "pong";
}

