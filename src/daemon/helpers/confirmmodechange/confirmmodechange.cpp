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

#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QIcon>

#include <KNotification>
#include <KLocalizedString>

ConfirmModeChange::ConfirmModeChange()
    : QObject()
{
    KNotification *notification = new KNotification(QStringLiteral("bluedevilConfirmModechange"),
                                                    KNotification::Persistent, this);

    notification->setText(i18nc(
        "Showed in a notification when the Bluetooth mode is going to be changed (for example to flight mode), the %1 is the name of the mode",
        "Change Bluetooth mode to '%1'?", qApp->arguments().at(1))
    );

    QStringList actions;
    actions.append(i18nc("Confirm the bluetooth mode change, shown in a notification button", "Confirm"));
    actions.append(i18nc("Deny the bluetooth mdoe change, shown in a notification", "Deny"));

    notification->setActions(actions);

    connect(notification, &KNotification::action1Activated,this, &ConfirmModeChange::confirm);
    connect(notification, &KNotification::action2Activated,this, &ConfirmModeChange::deny);
    connect(notification, &KNotification::closed, this, &ConfirmModeChange::deny);
    connect(notification, &KNotification::ignored, this, &ConfirmModeChange::deny);

    // We're using persistent notifications so we have to use our own timeout (10s)
    QTimer::singleShot(10000, notification, &KNotification::close);
    notification->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(42));
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
