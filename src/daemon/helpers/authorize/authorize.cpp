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

#include "authorize.h"

#include <QDebug>
#include <QCoreApplication>
#include <QIcon>

#include <KNotification>
#include <KLocalizedString>

Authorize::Authorize()
    : QObject()
{
    const QStringList &args = QCoreApplication::arguments();

    KNotification *notification = new KNotification(QStringLiteral("Authorize"),
                                                    KNotification::Persistent, this);

    notification->setText(i18nc(
        "Show a notification asking to authorize or deny access to this computer from Bluetooth. The %1 is the name of the bluetooth device",
        "%1 is requesting access to this computer", args.at(1))
    );

    QStringList actions;
    actions.append(i18nc("Button to trust a bluetooth remote device and authorize it to connect", "Trust and Authorize"));
    actions.append(i18nc("Button to authorize a bluetooth remote device to connect", "Authorize Only"));
    actions.append(i18nc("Deny access to a remote bluetooth device", "Deny"));

    notification->setActions(actions);

    connect(notification, &KNotification::action1Activated, this, &Authorize::trust);
    connect(notification, &KNotification::action2Activated, this, &Authorize::authorize);
    connect(notification, &KNotification::action3Activated, this, &Authorize::deny);
    connect(notification, &KNotification::closed, this, &Authorize::deny);
    connect(notification, &KNotification::ignored, this, &Authorize::deny);

    notification->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(42));
    notification->sendEvent();
}

void Authorize::authorize()
{
    qDebug() << "Accepted";
    QCoreApplication::exit(0);
}

void Authorize::trust()
{
    qDebug() << "Trusted";
    QCoreApplication::exit(1);
}

void Authorize::deny()
{
    qDebug() << "Rejected";
    QCoreApplication::exit(2);
}

