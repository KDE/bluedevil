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

#include <kapplication.h>

/**
 * @short Small class which send a KNotificaton to know if the Bluetooth device is authorized or not
 * A popup KNotification is send with 3 actions, trust accept and reject.
 * Trust set the remote device as trusted (using solid remote device) and quits with 0
 * Authorize quits the app with 0 (which means authorized).
 * Deny quits the app with 1 (which means denied)
 * @internal
 */
class Authorize : public QObject
{
    Q_OBJECT
    public:
        /**
         * Launch the KNotification which the respective actions, also makes the needed connection
         * between those actions and the slots
         */
        Authorize();

    private slots:
        void trust();
        void accept();
        void reject();
};
#endif
