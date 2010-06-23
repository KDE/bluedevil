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

#include "requestpin.h"
#include "ui_dialogWidget.h"

#include <iostream>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

#include <KIcon>
#include <knotification.h>
#include <klocale.h>
#include <kiconloader.h>
#include <KDialog>

RequestPin::RequestPin() : QObject()
{
    KNotification *notification = new KNotification("bluedevilRequestPin",
                                                    KNotification::Persistent, this);

    notification->setText(i18nc(
        "Showed in a notification to announce that a PIN is needed to acomplish a pair action, %1 is the name of the bluetooth device",
        "Pin is needed to pair with %1",qApp->arguments()[1])
    );

    QStringList actions;
    actions.append(i18nc(
        "Notification button which once clicked, a dialog to introduce the pin will be showed",
        "Introduce pin")
    );

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(introducePin()));
    connect(notification, SIGNAL(closed()), this, SLOT(deny()));
    connect(notification, SIGNAL(ignored()), this, SLOT(deny()));

    //We're using persistent notifications so we have to use our own timeout (10s)
    QTimer::singleShot(10000, notification, SLOT(close()));
    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    notification->sendEvent();
}

void RequestPin::introducePin()
{
    KIcon icon("preferences-system-bluetooth");

    Ui::dialogWidget *dialogWidget = new Ui::dialogWidget;
    QWidget *mainWidget = new QWidget();
    dialogWidget->setupUi(mainWidget);
    dialogWidget->descLabel->setText(i18nc(
        "Showed in a dialog which ask to introduce a PIN that will be used tu pair a bluetooth device, %1 is the name of the bluetooth device",
        "In order to pair this computer with %1 you've to enter a PIN, do it below please",
        qApp->arguments()[1])
    );
    dialogWidget->pixmap->setPixmap(icon.pixmap(64,64));

    KDialog *dialog = new KDialog();
    dialog->setMainWidget(mainWidget);

    dialog->setCaption(i18nc(
        "Showed in the caption of a dialog where the user introduce the PIN",
        "Introduce the PIN"
    ));

    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->setMinimumWidth(300);
    dialog->setMinimumHeight(150);
    dialog->setMaximumWidth(300);
    dialog->setMaximumHeight(150);

    if (dialog->exec() == KDialog::Accepted) {
        qApp->exit(0);
    }

    delete dialog;
    qApp->exit(1);
}

