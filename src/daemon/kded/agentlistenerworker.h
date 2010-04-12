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


#ifndef AGENTLISTENERWORKER_H
#define AGENTLISTENERWORKER_H

#include <QtDBus>
#include <QDebug>
#include <QThread>
#include <solid/control/bluetoothinterface.h>
/**
 * @internal
 * @short This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 *        from 2 QObjects
 * This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 * from 2 QObjects, so we had to create a new Class only to do the threading stuff
 * @ref AgentListenerWorker
 * @since 1.0
 */
class AgentListenerWorker : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent")

public:
    /**
     * Register the path and initialize the  m_adapter
     */
    AgentListenerWorker(QObject *parent);

public slots:
    /**
     * Called by bluez when another agent try to replace us as an agent
     */
    void Release();

    /**
     * Called by bluez to ask for a device authoritation
     */
    void Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg);

    /**
     * Called by bluez to ask for a PIN
     */
    QString RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg);

    /**
     * Called by bluez to ask for a passkey, currently is a aslias of RequestPinCode
     */
    quint32 RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg);

    /**
     * Called by bluez to display the passkey (Currently it's not implemented because we don't know
     * what to do with it).
     */
    void DisplayPasskey(QDBusObjectPath device, quint32 passkey);

    /**
     * Called by bluez to ask for a request confirmation
     */
    void RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg);

    /**
     * Called by bluez to confirm the change mode
     */
    void ConfirmModeChange(const QString& mode, const QDBusMessage &msg);

    /**
     * Called by bluez to inform that a process has failed (for example when the pin is introduced
     * too late and the device which ask for the pin is no longer listening).
     * We do anything here, since is not needed
     */
    void Cancel();

Q_SIGNALS:
    /**
     * Emited to propagate the release call (so BlueDevil can decide what to do)
     */
    void agentReleased();

public:
    /**
     * Called by agentListener just before delete. This is needed because ~QDBusAbstractAdaptor is
     * not virtual
     */
    void unregister();

private:
    /**
     * Unified method to return the bluez exception.
     * @param helper Name of the helper
     * @param msg The msg got from bluez
     */
    void sendBluezError(const QString& helper, const QDBusMessage &msg);

private:
    /**
     * Global adapter usually used to get information from a remote device
     */
    Solid::Control::BluetoothInterface *m_adapter;
};
#endif
