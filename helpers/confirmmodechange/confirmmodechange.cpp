/*  This file is part of the KDE project

    Copyright (C) 2010  Alex Fiestas <alex@eyeos.org>
    Copyright (C) 2010 by Eduardo Robles Elvira <edulix@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "confirmmodechange.h"
#include <knotification.h>
#include <klocale.h>

#include <QDebug>
#include <KIcon>
#include <kiconloader.h>
#include <QCoreApplication>
#include <solid/control/bluetoothmanager.h>

using namespace std;
ConfirmeModeChange::ConfirmeModeChange() : QObject()
{
    KNotification *notification = new KNotification("bluedevilConfirmModechange",
                                                        KNotification::Persistent |
                                                        KNotification::CloseWhenWidgetActivated,this);

    notification->setText(i18nc(
        "Showed in a notification when the Bluetooth mode is going to be changed (for example to flight mode), the %1 is the name of the mode",
        "Change bluetooth mode to %1 ?",qApp->arguments()[1])
    );
    QStringList actions;

    actions.append(i18nc("Confirm the bluetooth mode change, showed in a notification button", "Confirm"));
    actions.append(i18nc("Deny the bluetooth mdoe change, showed in a notification", "Deny"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(confirm()));
    connect(notification, SIGNAL(action2Activated()),this, SLOT(deny()));

    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    notification->sendEvent();
}

void ConfirmeModeChange::confirm()
{
    qDebug << "confirmed";
    qApp->exit(0);
}

void ConfirmeModeChange::deny()
{
    qDebug() << "Denied";
    qApp->exit(1);
}

