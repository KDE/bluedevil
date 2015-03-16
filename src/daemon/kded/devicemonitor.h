/***************************************************************************
 *   Copyright (C) 2015 David Rosca <nowrep@gmail.com>                     *
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

#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QObject>

#include <KSharedConfig>

namespace BlueDevil
{
    class Manager;
    class Adapter;
    class Device;
}

class KFilePlacesModel;

class DeviceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit DeviceMonitor(QObject *parent = 0);
    ~DeviceMonitor();

private Q_SLOTS:
    void usableAdapterChanged(BlueDevil::Adapter *adapter);
    void adapterAdded(BlueDevil::Adapter *adapter);
    void deviceAdded(BlueDevil::Device *device);

    void deviceConnectedChanged(bool connected);
    void login1PrepareForSleep(bool active);

private:
    void saveState();
    void restoreState();
    void restoreAdapter(BlueDevil::Adapter *adapter);

    void clearPlaces();
    void updateDevicePlace(BlueDevil::Device *device);

    BlueDevil::Device *deviceForAddress(const QString &address) const;

    BlueDevil::Manager *m_manager;
    KFilePlacesModel *m_places;
    KSharedConfig::Ptr m_config;
};

#endif // DEVICEMONITOR_H
