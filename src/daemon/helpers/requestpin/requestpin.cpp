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

#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QIcon>
#include <QDialog>
#include <QPushButton>

#include <KNotification>
#include <KLocalizedString>

RequestPin::RequestPin() : QObject()
    , m_dialogWidget(0)
{
    m_notification = new KNotification(QStringLiteral("bluedevilRequestPin"),
                                       KNotification::Persistent, this);

    m_notification->setText(i18nc(
        "Shown in a notification to announce that a PIN is needed to accomplish a pair action, %1 is the name of the bluetooth device",
        "PIN needed to pair with %1", qApp->arguments().at(1))
    );

    QStringList actions;
    actions.append(i18nc(
        "Notification button which once clicked, a dialog to introduce the PIN will be shown",
        "Introduce PIN")
    );

    m_notification->setActions(actions);

    connect(m_notification, &KNotification::action1Activated,this, &RequestPin::introducePin);
    connect(m_notification, &KNotification::closed, this, &RequestPin::quit);
    connect(m_notification, &KNotification::ignored, this, &RequestPin::quit);

    // We're using persistent notifications so we have to use our own timeout (10s)
    m_timer.setSingleShot(true);
    m_timer.setInterval(10000);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, m_notification, &KNotification::close);

    m_notification->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(42));
    m_notification->sendEvent();
}

void RequestPin::introducePin()
{
    m_timer.stop();
    m_notification->disconnect();
    m_notification->close();
    m_notification->deleteLater();

    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(i18nc(
        "Shown in the caption of a dialog where the user introduce the PIN",
        "Introduce PIN"
    ));

    m_dialogWidget = new Ui::dialogWidget;
    m_dialogWidget->setupUi(dialog);
    m_dialogWidget->descLabel->setText(i18nc(
        "Shown in a dialog which asks to introduce a PIN that will be used to pair a Bluetooth device, %1 is the name of the Bluetooth device",
        "In order to pair this computer with %1, you have to enter a PIN. Please do it below.",
        qApp->arguments().at(1))
    );
    m_dialogWidget->pixmap->setPixmap(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")).pixmap(64));

    connect(m_dialogWidget->pin, &KLineEdit::textChanged, this, &RequestPin::checkPin);
    connect(m_dialogWidget->pin, &KLineEdit::returnPressed, dialog, &QDialog::accept);

    m_dialogWidget->pin->setFocus(Qt::ActiveWindowFocusReason);

    if (qApp->arguments().count() > 2 && qApp->arguments().at(2) == QLatin1String("numeric")) {
        m_dialogWidget->pin->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[0-9]{1,6}")), this));
    } else {
        m_dialogWidget->pin->setValidator(new QRegExpValidator(QRegExp(QStringLiteral("[A-Za-z0-9]{1,16}")), this));
    }

    m_dialogWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialog->setMinimumSize(dialog->sizeHint());
    dialog->setMaximumSize(dialog->sizeHint());
    if (dialog->exec()) {
        std::cout << m_dialogWidget->pin->text().toLatin1().constData();
        std::flush(std::cout);
        delete dialog;
        qApp->exit(0);
        return;
    }

    delete dialog;
    qApp->exit(1);
}

void RequestPin::checkPin(const QString &pin)
{
    m_dialogWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!pin.isEmpty());
}

void RequestPin::quit()
{
    qApp->exit(1);
}
