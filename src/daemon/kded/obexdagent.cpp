/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "obexdagent.h"
#include "filereceiversettings.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusMessage>

#include <KDebug>
#include <KNotification>
#include <KLocale>
#include <KIcon>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;
ObexdAgent::ObexdAgent(QObject* parent) : QDBusAbstractAdaptor(parent)
{
    if (!QDBusConnection::sessionBus().registerObject("/BlueDevil_obexdAgent", parent)) {
        kDebug() << "The dbus object can't be registered";
        return;
    }
}


QString ObexdAgent::Authorize(const QDBusObjectPath &transfer, const QString &bt_address,
                    const QString &name, const QString &type, int length, int time, const QDBusMessage &msg)
{
    kDebug() << "Authorize";
    if (FileReceiverSettings::self()->autoAccept()) {
        kDebug() << "Auto accepting";
        return FileReceiverSettings::self()->saveUrl().path();
    }

    m_pendingMessage = msg;
    m_pendingMessage.setDelayedReply(true);

    m_notification = new KNotification("bluedevilIncomingFile",
       KNotification::Persistent, this);

    Device *device = Manager::self()->defaultAdapter()->deviceForAddress(bt_address);
    m_notification->setText(i18nc(
        "Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
        "%1 is sending you the file %2", device->name(), name));
    QStringList actions;

    actions.append(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    actions.append(i18nc("Button to accept the incoming file transfer and show a Save as... dialog that will let the user choose where will the file be downloaded to", "Save as..."));
    actions.append(i18nc("Deny the incoming file transfer", "Cancel"));

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()), this, SLOT(fileAccepted()));
    connect(m_notification, SIGNAL(action2Activated()), this, SLOT(fileSaveAs()));
    connect(m_notification, SIGNAL(action3Activated()), this, SLOT(fileCanceled()));

    m_notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42, 42));
    m_notification->setComponentData(KComponentData("bluedevil"));
    m_notification->sendEvent();

    return QString();
}

void ObexdAgent::Cancel()
{
    kDebug() << "AAAAAAAA";
}

void ObexdAgent::fileAccepted()
{
    kDebug();
    QDBusMessage msg = m_pendingMessage.createReply(QVariant::fromValue<QString>("/tmp/rolf.txt"));
    QDBusConnection::sessionBus().send(msg);
}

void ObexdAgent::fileSaveAs()
{
    //this will open an external process
}

void ObexdAgent::fileCanceled()
{
    kDebug();
    QDBusMessage error = m_pendingMessage.createErrorReply("org.openobex.Error.Rejected", "Pincode request failed");
    QDBusConnection::sessionBus().send(error);
}
