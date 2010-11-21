/***************************************************************************
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@eyeos.org>        *
 *   Copyright (C) 2010 Eduardo Robles Elvira <edulix@gmail.com>           *
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

#include "confirmmodechange.h"

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

#include <KIcon>
#include <kiconloader.h>
#include <knotification.h>
#include <klocale.h>

ConfirmModeChange::ConfirmModeChange()
    : QObject()
{
    KNotification *notification = new KNotification("bluedevilConfirmModechange",
                                                    KNotification::Persistent, this);

    notification->setText(i18nc(
        "Showed in a notification when the Bluetooth mode is going to be changed (for example to flight mode), the %1 is the name of the mode",
        "Change Bluetooth mode to '%1'?", qApp->arguments()[1])
    );

    QStringList actions;
    actions.append(i18nc("Confirm the bluetooth mode change, shown in a notification button", "Confirm"));
    actions.append(i18nc("Deny the bluetooth mdoe change, shown in a notification", "Deny"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(confirm()));
    connect(notification, SIGNAL(action2Activated()),this, SLOT(deny()));
    connect(notification, SIGNAL(closed()), this, SLOT(deny()));
    connect(notification, SIGNAL(ignored()), this, SLOT(deny()));

    // We're using persistent notifications so we have to use our own timeout (10s)
    QTimer::singleShot(10000, notification, SLOT(close()));
    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42, 42));
    notification->sendEvent();
}

void ConfirmModeChange::confirm()
{
    qDebug() << "confirmed";
    qApp->exit(0);
}

void ConfirmModeChange::deny()
{
    qDebug() << "Denied";
    qApp->exit(1);
}

