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

#ifndef REQUESTPIN_H
#define REQUESTPIN_H

#include <QtCore/QObject>
#include <QTimer>

class KDialog;
class KNotification;

/**
 * @short Small class which send a KNotificaton to know if the Bluetooth device is authorized or not
 * A popup KNotification is send with 3 actions, trust accept and reject.
 * Trust set the remote device as trusted (using libbluedevil remote device) and quits with 0
 * Authorize quits the app with 0 (which means authorized).
 * Deny quits the app with 1 (which means denied)
 * @internal
 */
class RequestPin
    : public QObject
{
    Q_OBJECT

public:
    /**
     * Launch the KNotification which the respective actions, also makes the needed connection
     * between those actions and the slots
     */
    RequestPin();

private Q_SLOTS:
    /**
     * Show a dialog with widgetDialog as mainWidget where the user will write the PIN code.
     * If the user click the button 1, the app print the PIN and quits the app as success
     * In case of button 2, the app is quit as error
     */
    void introducePin();

    /**
     * If the notification is ignored or closed, then we have to quit the helper
     */
    void quit();
    void checkPin(const QString &pin);

private:
    QTimer m_timer;
    KDialog *m_dialog;
    KNotification *m_notification;
};
#endif //REQUESTPIN_H
