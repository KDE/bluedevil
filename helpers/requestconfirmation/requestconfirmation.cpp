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

#include "requestconfirmation.h"
#include <knotification.h>
#include <klocale.h>

#include <QDebug>
#include <KIcon>
#include <kiconloader.h>
#include <QCoreApplication>
#include <solid/control/bluetoothmanager.h>

RequestConfirmation::RequestConfirmation() : QObject()
{
    KNotification *notification = new KNotification("bluedevilRequestConfirmation",
                                                        KNotification::Persistent |
                                                        KNotification::CloseWhenWidgetActivated,this);

    notification->setText(i18nc(
        "The text is showed in a knotification to know if the PIN is correct, %1 is the remote bluetotoh device and %2 is the pin",
        "%1 is asking if the PIN is correct: %2", qApp->arguments()[1], qApp->arguments()[2])
    );

    QStringList actions;
    actions.append(i18nc("Notification button to know if the pin is correct or not", "Correct pin"));
    actions.append(i18nc("Notification button to say that the PIN is wrong", "Wrong pin"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(pinCorrect()));
    connect(notification, SIGNAL(action2Activated()),this, SLOT(pinWrong()));

    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    notification->sendEvent();
}

void RequestConfirmation::pinCorrect()
{
    qApp->exit(0);
}

void RequestConfirmation::pinWrong()
{
    qApp->exit(1);
}

