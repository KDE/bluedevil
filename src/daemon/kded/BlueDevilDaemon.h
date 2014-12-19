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

#ifndef BLUEDEVILDAEMON_H
#define BLUEDEVILDAEMON_H

#include <kdedmodule.h>
#include <QMap>
#include <QStringList>

typedef QMap <QString, QString> DeviceInfo;
typedef QMap<QString, DeviceInfo > QMapDeviceInfo;

class QDBusPendingCallWatcher;
namespace BlueDevil {
    class Adapter;
    class Device;
};
using namespace BlueDevil;

class KDE_EXPORT BlueDevilDaemon
    : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil")

public:
    /**
     * Stablish basics connections with libbluedevil signals and calls online if interfaces are availables
     */
    BlueDevilDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~BlueDevilDaemon();

public Q_SLOTS:
    /**
     * Returns whether the daemon is in online mode (eg. Bluez services are
     * running and we have usable adapter)
     */
    Q_SCRIPTABLE bool isOnline();

    /**
     * Returns QMap<Address, DeviceInfo> with all known devices
     */
    Q_SCRIPTABLE QMapDeviceInfo allDevices();

    /**
     * Returns DeviceInfo for one device.
     */
    Q_SCRIPTABLE DeviceInfo device(const QString &address);

    /**
     * Starts discovery for timeout miliseconds (0 = forever)
     */
    Q_SCRIPTABLE void startDiscovering(quint32 timeout);

    /**
     * Stops discovery (if it was previously started)
     */
    Q_SCRIPTABLE void stopDiscovering();

private:
    /**
     * Called by constructor or eventually by adapterAdded initialize all the helpers
     * @see helpers
     */
    void onlineMode();

    /**
     * Called eventually adapterRemoved shutdown all the helpers
     * @see helpers
     */
    void offlineMode();

private Q_SLOTS:
    /**
     * Called when the default adapter changes, re-initialize the kded with the new
     * default adapter
     */
    void usableAdapterChanged(Adapter *adapter);

    void adapterAdded(Adapter *adapter);
    void adapterRemoved(Adapter *adapter);

    /**
     * When the agent is released this is called to unload it
     */
    void agentReleased();

    void login1PrepareForSleep(bool active);

    void deviceFound(Device*);
    void monolithicQuit(QDBusPendingCallWatcher* watcher);
    void monolithicFinished(const QString &);

private:
    void executeMonolithic();
    void killMonolithic();

    void saveAdaptersState();
    void restoreAdaptersState();
    void restoreAdapterState(Adapter *adapter);

    DeviceInfo deviceToInfo(Device *const device) const;

private:
    struct Private;
    Private *d;
};

extern int dblue();
#endif /*BLUEDEVILDAEMON_H*/
