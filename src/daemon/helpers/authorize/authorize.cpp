/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "authorize.h"
#include <knotification.h>
#include <klocale.h>

#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

#include <KIcon>
#include <kiconloader.h>
#include <solid/control/bluetoothmanager.h>

using namespace std;
Authorize::Authorize() : QObject()
{
    KNotification *notification = new KNotification("bluedevilAuthorize",
                                                        KNotification::Persistent,this);

    notification->setText(i18nc(
        "Show a notification asking for authorize or deny access to this computer from Bluetooth the %1 is the name of the bluetooth device",
        "%1 is requesting access to this computer",qApp->arguments()[1])
    );
    QStringList actions;

    actions.append(i18nc("Button to trust a bluetooth remote device and authorize it to connect", "Trust"));
    actions.append(i18nc("Button to authorize a bluetooth remote device to connect ", "Authorize"));
    actions.append(i18nc("Deny access to a remote bluetooth device", "Deny"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(trust()));
    connect(notification, SIGNAL(action2Activated()),this, SLOT(authorize()));
    connect(notification, SIGNAL(action3Activated()),this, SLOT(deny()));
    connect(notification, SIGNAL(closed()), this, SLOT(deny()));
    connect(notification, SIGNAL(ignored()), this, SLOT(deny()));

    notification->setPixmap(KIcon(qApp->arguments()[2]).pixmap(52,52));

    //We're using persistent notifications so we have to use our own timeout (10s)
    QTimer::singleShot(10000, notification, SLOT(close()));
    notification->sendEvent();
}

void Authorize::trust()
{
    qDebug() << "Trusted";
    Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface *adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
    Solid::Control::BluetoothRemoteDevice *remoteDevice = adapter->findBluetoothRemoteDeviceUBI(qApp->arguments().last());
    remoteDevice->setTrusted(true);
    qApp->exit(0);
}

void Authorize::authorize()
{
    qDebug() << "Accepted";
    qApp->exit(0);
}

void Authorize::deny()
{
    qDebug() << "Rejected";
    qApp->exit(1);
}

