/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *   Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>                *
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

#include "requestauthorization.h"
#include "debug_p.h"

#include <QIcon>

#include <KNotification>
#include <KLocalizedString>

RequestAuthorization::RequestAuthorization(BluezQt::DevicePtr device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{
    KNotification *notification = new KNotification(QStringLiteral("Authorize"),
                                                    KNotification::Persistent, this);

    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(QStringLiteral("%1 (%2)").arg(m_device->name(), m_device->address()));
    notification->setText(i18nc("Show a notification asking to authorize or deny access to this computer from Bluetooth."
                                "The %1 is the name of the bluetooth device",
                                "%1 is requesting access to this computer", m_device->name()));

    QStringList actions;
    actions.append(i18nc("Button to trust a bluetooth remote device and authorize it to connect", "Trust && Authorize"));
    actions.append(i18nc("Button to authorize a bluetooth remote device to connect", "Authorize Only"));
    actions.append(i18nc("Deny access to a remote bluetooth device", "Deny"));

    notification->setActions(actions);

    connect(notification, &KNotification::action1Activated, this, &RequestAuthorization::authorizeAndTrust);
    connect(notification, &KNotification::action2Activated, this, &RequestAuthorization::authorize);
    connect(notification, &KNotification::action3Activated, this, &RequestAuthorization::deny);
    connect(notification, &KNotification::closed, this, &RequestAuthorization::deny);
    connect(notification, &KNotification::ignored, this, &RequestAuthorization::deny);
    connect(parent, SIGNAL(agentCanceled()), this, SLOT(deny()));

    notification->sendEvent();
}

void RequestAuthorization::authorizeAndTrust()
{
    qCDebug(BLUEDAEMON) << "Authorization accepted and trusted:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(AcceptAndTrust);
}

void RequestAuthorization::authorize()
{
    qCDebug(BLUEDAEMON) << "Authorization accepted:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(Accept);
}

void RequestAuthorization::deny()
{
    qCDebug(BLUEDAEMON) << "Authorization denied:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(Deny);
}

