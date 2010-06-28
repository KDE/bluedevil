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

#ifndef AUTHORIZE_H
#define AUTHORIZE_H

#include <QtCore/QObject>

/**
 * @short Small class which send a KNotificaton to know if the mode change is authorized or not
 * A popup KNotification is send with 3 actions, trust accept and reject.
 * Trust set the remote device as trusted (using libbluedevil remote device) and quits with 0
 * Authorize quits the app with 0 (which means authorized).
 * Deny quits the app with 1 (which means denied)
 * @internal
 */
class ConfirmModeChange
    : public QObject
{
    Q_OBJECT

public:
    /**
     * Launch the KNotification which the respective actions, also makes the needed connection
     * between those actions and the slots
     */
    ConfirmModeChange();

private Q_SLOTS:
    /**
     * Quits the application as success
     */
    void confirm();

    /**
     * Quits the application as error
     */
    void deny();
};
#endif
