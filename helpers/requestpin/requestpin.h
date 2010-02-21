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

#ifndef AUTHORIZE_H
#define AUTHORIZE_H

#include <QObject>

/**
 * @short Small class which send a KNotificaton to know if the Bluetooth device is authorized or not
 * A popup KNotification is send with 3 actions, trust accept and reject.
 * Trust set the remote device as trusted (using solid remote device) and quits with 0
 * Authorize quits the app with 0 (which means authorized).
 * Deny quits the app with 1 (which means denied)
 * @internal
 */
class RequestPin : public QObject
{
    Q_OBJECT
    public:
        /**
         * Launch the KNotification which the respective actions, also makes the needed connection
         * between those actions and the slots
         */
        RequestPin();

    private slots:
        /**
         * Show a dialog with widgetDialog as mainWidget where the user will write the PIN code.
         * If the user click the button 1, the app print the PIN and quits the app as success
         * In case of button 2, the app is quit as error
         */
        void introducePin();
};
#endif
