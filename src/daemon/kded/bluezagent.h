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

#ifndef BLUEZAGENT_H
#define BLUEZAGENT_H

#include <QBluez/Agent>

class QProcess;

class BluezAgent : public QBluez::Agent
{
    Q_OBJECT

public:
    /**
     * Register the path and initialize the  m_adapter
     */
    BluezAgent(QObject *parent);

    QDBusObjectPath objectPath() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    /**
     * Called by bluez when another agent try to replace us as an agent
     */
    void release() Q_DECL_OVERRIDE;

    /**
     * Called by bluez to ask for a device authoritation
     */
    void authorizeService(QBluez::Device *device, const QString &uuid, const QBluez::Request<void> &request) Q_DECL_OVERRIDE;

    /**
     * Called by bluez to ask for a PIN
     */
    void requestPinCode(QBluez::Device *device, const QBluez::Request<QString> &request) Q_DECL_OVERRIDE;

    /**
     * Called by bluez to ask for a passkey, currently is a aslias of RequestPinCode
     */
    void requestPasskey(QBluez::Device *device, const QBluez::Request<quint32> &request) Q_DECL_OVERRIDE;

    /**
     * Called by bluez to display the passkey (Currently it's not implemented because we don't know
     * what to do with it).
     */
    void displayPasskey(QBluez::Device *device, const QString &passkey, const QString &entered) Q_DECL_OVERRIDE;

    /**
     * Called by bluez to ask for a request confirmation
     */
    void requestConfirmation(QBluez::Device *device, const QString &passkey, const QBluez::Request<void> &request) Q_DECL_OVERRIDE;

    /**
     * Called by bluez to inform that a process has failed (for example when the pin is introduced
     * too late and the device which ask for the pin is no longer listening).
     * We do anything here, since is not needed
     */
    void cancel() Q_DECL_OVERRIDE;

    /**
     * Slot for those calls that should return a Bool result
     *
     * This slot gets called when the helper process ends, and basically checks the exitCode
     */
    void processClosedBool(int exitCode);

    /**
     * Slot for authorize service call
     *
     * This slot gets called when the helper process ends, basically checks the exitCode and
     * set the device as trusted.
     */
    void processClosedAuthorize(int exitCode);

    /**
     * Just like @processClosedBool but this instead returns a String (the PIN)
     *
     * This slot gets called when the RequestPin helper ends
     */
    void processClosedPin(int exitCode);

    /**
     * Just like @processClosedBool but this instead returns a quint32 (the PIN)
     *
     * This slot gets called when the RequestPasskey helper ends
     */
    void processClosedPasskey(int exitCode);

Q_SIGNALS:
    /**
     * Emited to propagate the release call (so BlueDevil can decide what to do)
     */
    void agentReleased();

private:
    QProcess *m_process;
    QBluez::Device *m_device;
    QBluez::Request<> m_boolRequest;
    QBluez::Request<quint32> m_passkeyRequest;
    QBluez::Request<QString> m_pinRequest;

};
#endif
