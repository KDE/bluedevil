/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "requestauthorization.h"
#include "debug_p.h"

#include <QIcon>

#include <KLocalizedString>
#include <KNotification>

RequestAuthorization::RequestAuthorization(BluezQt::DevicePtr device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{
    KNotification *notification = new KNotification(QStringLiteral("Authorize"), KNotification::Persistent, this);

    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(QStringLiteral("%1 (%2)").arg(m_device->name().toHtmlEscaped(), m_device->address().toHtmlEscaped()));
    notification->setText(
        i18nc("Show a notification asking to authorize or deny access to this computer from Bluetooth."
              "The %1 is the name of the bluetooth device",
              "%1 is requesting access to this computer",
              m_device->name().toHtmlEscaped()));

    QStringList actions;
    actions.append(i18nc("Button to trust a bluetooth remote device and authorize it to connect", "Trust and Authorize"));
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
