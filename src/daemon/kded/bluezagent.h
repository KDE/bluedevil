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

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusAbstractAdaptor>

class QProcess;
namespace BlueDevil {
    class Adapter;
};

/**
 * @internal
 * @short This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 *        from 2 QObjects
 * This class is only a delegate to be able to use agentlistener on a QThread (We can't inherit
 * from 2 QObjects, so we had to create a new Class only to do the threading stuff
 * @ref AgentListenerWorker
 * @since 1.0
 */
class BluezAgent
    : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    /**
     * Register the path and initialize the  m_adapter
     */
    BluezAgent(QObject *parent);

public Q_SLOTS:
    /**
     * Called by bluez when another agent try to replace us as an agent
     */
    void Release();

    /**
     * Called by bluez to ask for a device authoritation
     */
    void AuthorizeService(const QDBusObjectPath &device, const QString& uuid, const QDBusMessage &msg);

    /**
     * Called by bluez to ask for a PIN
     */
    QString RequestPinCode(const QDBusObjectPath& device, const QDBusMessage& msg);

    /**
     * Called by bluez to ask for a passkey, currently is a aslias of RequestPinCode
     */
    quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &msg);

    /**
     * Called by bluez to display the passkey (Currently it's not implemented because we don't know
     * what to do with it).
     */
    void DisplayPasskey(const QDBusObjectPath &device, quint32 passkey);

    /**
     * Called by bluez to ask for a request confirmation
     */
    void RequestConfirmation(const QDBusObjectPath &device, quint32 passkey, const QDBusMessage &msg);

    /**
     * Called by bluez to inform that a process has failed (for example when the pin is introduced
     * too late and the device which ask for the pin is no longer listening).
     * We do anything here, since is not needed
     */
    void Cancel();

    /**
     * Slot for those calls that should return a Bool result
     *
     * This slot gets called when the helper process ends, and basically checks the exitCode
     */
    void processClosedBool(int exitCode);

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

    /**
     * Returns the name of the device if it is registered on the bus
     *
     * @param UBI of the device
     * @return the device->name() or "Bluetooth" if device doesn't exists;
     */
    QString deviceName(const QString &UBI);

private:
    QProcess           *m_process;
    QDBusMessage        m_msg;
    QString             m_currentHelper;
};
#endif
