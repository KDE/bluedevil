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
#include <kdebug.h>
#include "openobex/serversession.h"

Service::Service()
{
    kDebug(4567);
    new ServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/Service", this);
    dbus.registerService("org.kde.BlueDevil.Service");
    m_serversession = 0;
}

Service::~Service()
{
    m_serversession->deleteLater();
}

void Service::errorOccured(const QString& errorName, const QString& errorMessage)
{
    kDebug(4567) << "errorName" << errorName << "errorMessage" << errorMessage;
    if (m_serversession) {
        m_serversession->deleteLater();
    }
}

void Service::sessionCreated(const QDBusObjectPath& path)
{
    // TODO: Be able to deal with more than one session at a time
    m_serversession = new OpenObex::ServerSession(path.path());
}

void Service::sessionRemoved(const QDBusObjectPath& path)
{
    m_serversession->deleteLater();
    m_serversession = 0;
}

QString Service::ping()
{
    return "pong";
}

