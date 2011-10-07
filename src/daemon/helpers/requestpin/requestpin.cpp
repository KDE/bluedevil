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

using namespace std;
RequestPin::RequestPin() : QObject()
{
    m_notification = new KNotification("bluedevilRequestPin",
                                       KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Shown in a notification to announce that a PIN is needed to accomplish a pair action, %1 is the name of the bluetooth device",
        "PIN needed to pair with %1",qApp->arguments()[1])
    );

    QStringList actions;
    actions.append(i18nc(
        "Notification button which once clicked, a dialog to introduce the PIN will be shown",
        "Introduce PIN")
    );

    m_notification->setActions(actions);

    connect(m_notification, SIGNAL(action1Activated()),this, SLOT(introducePin()));
    connect(m_notification, SIGNAL(closed()), this, SLOT(quit()));
    connect(m_notification, SIGNAL(ignored()), this, SLOT(quit()));

    //We're using persistent notifications so we have to use our own timeout (10s)
    QTimer::singleShot(10000, m_notification, SLOT(close()));
    m_notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    m_notification->sendEvent();
}

void RequestPin::introducePin()
{
    disconnect(m_notification, SIGNAL(closed()), this, SLOT(quit()));
    disconnect(m_notification, SIGNAL(ignored()), this, SLOT(quit()));

    KIcon icon("preferences-system-bluetooth");

    Ui::dialogWidget *dialogWidget = new Ui::dialogWidget;
    QWidget *mainWidget = new QWidget();
    dialogWidget->setupUi(mainWidget);
    dialogWidget->descLabel->setText(i18nc(
        "Shown in a dialog which asks to introduce a PIN that will be used to pair a Bluetooth device, %1 is the name of the Bluetooth device",
        "In order to pair this computer with %1, you have to enter a PIN. Please do it below.",
        qApp->arguments()[1])
    );
    dialogWidget->pixmap->setPixmap(icon.pixmap(64,64));

    KDialog *dialog = new KDialog();
    dialog->setMainWidget(mainWidget);

    dialog->setCaption(i18nc(
        "Shown in the caption of a dialog where the user introduce the PIN",
        "Introduce PIN"
    ));

    QObject::connect(dialogWidget->pin, SIGNAL(returnPressed()),
                     dialog, SLOT(accept()));

    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->setMinimumWidth(300);
    dialog->setMinimumHeight(150);
    dialog->setMaximumWidth(300);
    dialog->setMaximumHeight(150);

    dialogWidget->pin->setFocus(Qt::ActiveWindowFocusReason);

    if (dialog->exec()) {
        cout << dialogWidget->pin->text().toLatin1().data();
        flush(cout);
        qApp->exit(0);
        return;
    }

    delete dialog;
    qApp->exit(1);
}

void RequestPin::quit()
{
    qApp->exit(1);
}
