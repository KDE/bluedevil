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

#include <KDEDModule>
#include <QLoggingCategory>

typedef QMap<QString, QString> DeviceInfo;
typedef QMap<QString, DeviceInfo> QMapDeviceInfo;

class QDBusPendingCallWatcher;

namespace QBluez {
    class Adapter;
    class Device;
}

class Q_DECL_EXPORT BlueDevilDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil")

public:
    /**
     * Establish basics connections with QBluez signals and calls online if interfaces are availables
     */
    BlueDevilDaemon(QObject *parent, const QList<QVariant>&);
    ~BlueDevilDaemon();

public Q_SLOTS:
    Q_SCRIPTABLE bool isOnline();

    /**
     * This slot will return a list of devices made of: configured and discovered devices.
     * Going deeper, the first time that this slot is called a discovery of X seconds will start
     * Then if this slot is consulted again it will return configured and discovered device. Once
     * the discovery ends it won't start a new discovery until N seconds pass.
     */
    Q_SCRIPTABLE QMapDeviceInfo knownDevices();

    /**
     * Returns DeviceInfo for one device.
     */
    Q_SCRIPTABLE DeviceInfo deviceInfo(const QString &address);

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
    void usableAdapterChanged(QBluez::Adapter *adapter);

    /**
     * When the agent is released this is called to unload it
     */
    void agentReleased();

    void deviceFound(QBluez::Device*);
    void monolithicQuit(QDBusPendingCallWatcher *watcher);
    void monolithicFinished(const QString &);

private:
    void executeMonolithic();
    void killMonolithic();

    DeviceInfo deviceToInfo (QBluez::Device *const device) const;

private:
    struct Private;
    Private *d;
};

#endif /*BLUEDEVILDAEMON_H*/
