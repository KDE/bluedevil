/***************************************************************************
 *   Copyright (C) 2008  Alex Fiestas <alex@eyeos.org>                     *
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
#include <KIcon>
#include <kiconloader.h>
#include <QCoreApplication>
#include <iostream>
#include <solid/control/bluetoothmanager.h>

using namespace std;
Authorize::Authorize() : QObject()
{
    KNotification *notification = new KNotification("bluedevilAuthorize",
                                                        KNotification::Persistent |
                                                        KNotification::CloseWhenWidgetActivated,this);

    notification->setText(i18n("%1 is requesting access to this computer",qApp->arguments()[1]));
    QStringList actions;

    actions.append(i18nc("Text of button to always trust a bluetooth device", "Trust"));
    actions.append(i18nc("Text of button to access a conneciton only once", "Accept"));
    actions.append(i18nc("Text of button to reject the incoming bluetooth connection", "Reject"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(trust()));
    connect(notification, SIGNAL(action2Activated()),this, SLOT(accept()));
    connect(notification, SIGNAL(action3Activated()),this, SLOT(reject()));

    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    notification->sendEvent();
    qDebug() << "Sending the F**** notification";
}

void Authorize::trust()
{
    qDebug() << "Trusted";
    Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface *adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
    Solid::Control::BluetoothRemoteDevice *remoteDevice = adapter->findBluetoothRemoteDeviceUBI(qApp->arguments()[2]);
    remoteDevice->setTrusted(true);
    qApp->exit(0);
}

void Authorize::accept()
{
    cout << "Accepted";
    qApp->exit(0);
}

void Authorize::reject()
{
    qDebug() << "Rejected";
    qApp->exit(1);
}

