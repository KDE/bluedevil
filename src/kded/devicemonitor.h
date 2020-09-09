/*
 *   SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QObject>

#include <KSharedConfig>

#include <BluezQt/Types>

class KFilePlacesModel;

class BlueDevilDaemon;

class DeviceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit DeviceMonitor(BlueDevilDaemon *daemon);

    void saveState();

private Q_SLOTS:
    void bluetoothOperationalChanged(bool operational);
    void adapterAdded(BluezQt::AdapterPtr adapter);
    void deviceAdded(BluezQt::DevicePtr device);

    void deviceConnectedChanged(bool connected);
    void login1PrepareForSleep(bool active);

private:
    void restoreState();
    void restoreAdapter(BluezQt::AdapterPtr adapter);

    void clearPlaces();
    void updateDevicePlace(BluezQt::DevicePtr device);

    KFilePlacesModel *places();

    BluezQt::Manager *m_manager;
    KFilePlacesModel *m_places = nullptr;
    KSharedConfig::Ptr m_config;
};

#endif // DEVICEMONITOR_H
