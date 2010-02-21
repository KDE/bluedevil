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

#include "requestpin.h"
#include <knotification.h>
#include "ui_dialogWidget.h"
#include <klocale.h>

#include <QDebug>
#include <KIcon>
#include <kiconloader.h>
#include <QCoreApplication>
#include <iostream>
#include <solid/control/bluetoothmanager.h>
#include <KDialog>

using namespace std;
RequestPin::RequestPin() : QObject()
{
    KNotification *notification = new KNotification("bluedevilRequestPin",
                                                        KNotification::Persistent |
                                                        KNotification::CloseWhenWidgetActivated,this);

    notification->setText(i18n("Pin is needed to pair with %1",qApp->arguments()[1]));
    QStringList actions;

    actions.append(i18nc("Text of button to always trust a bluetooth device", "Introduce pin"));

    notification->setActions(actions);

    connect(notification, SIGNAL(action1Activated()),this, SLOT(introducePin()));

    notification->setPixmap(KIcon("preferences-system-bluetooth").pixmap(42,42));
    notification->sendEvent();
}

void RequestPin::introducePin()
{
    KIcon icon = KIcon("preferences-system-bluetooth");

    Ui::dialogWidget *dialogWidget = new Ui::dialogWidget;
    QWidget *mainWidget = new QWidget();
    dialogWidget->setupUi(mainWidget);
    dialogWidget->descLabel->setText(i18n("In order to pair this computer with %1 you've to enter a PIN, do it below please",qApp->arguments()[1]));
    dialogWidget->pixmap->setPixmap(icon.pixmap(64,64));
    
    KDialog *dialog = new KDialog();
    dialog->setMainWidget(mainWidget);

    dialog->setCaption(i18n("Introduce the PIN"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->setMinimumWidth(300);
    dialog->setMinimumHeight(150);
    dialog->setMaximumWidth(300);
    dialog->setMaximumHeight(150);
    int response = dialog->exec();

    if(response == QDialog::Accepted)
    {
        cout << dialogWidget->pin->text().toLatin1().data();
        qApp->exit(0);
    }
    delete dialog;
    qApp->exit(1);
}

