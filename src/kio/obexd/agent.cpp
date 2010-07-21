/*  This file is part of the KDE libraries

    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "agent.h"
#include "agentadaptor.h"

Agent::Agent(QObject *parent)
    : QObject(parent)
{
    new AgentAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/", this);
    dbus.registerService("org.openobex.agent");
}

Agent::~Agent()
{
}

QString Agent::Authorize(const QDBusObjectPath &in0, const QString &in1, const QString &in2, const QString &in3, int in4, int in5)
{
    Q_UNUSED(in0)
    Q_UNUSED(in1)
    Q_UNUSED(in2)
    Q_UNUSED(in3)
    Q_UNUSED(in4)
    Q_UNUSED(in5)

    return QString();
}

void Agent::Cancel()
{
}
