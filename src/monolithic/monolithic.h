/*
    Copyright (C) 2010 Alejandro Fiestas Olivares  <alex@ufocoders.com>
    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MONOLITHIC_H
#define MONOLITHIC_H

#include <QtCore/QMultiMap>

#include <kstatusnotifieritem.h>

namespace BlueDevil {
    class Adapter;
    class Device;
}

class QAction;
class KAction;

using namespace BlueDevil;

class Monolithic
    : public KStatusNotifierItem
{

Q_OBJECT
public:
    Monolithic(QObject* parent = 0);

    struct EntryInfo {
        Device *device;
        QString service;
        void   *dbusService;
    };

public Q_SLOTS:
    void adapterChanged();

    void regenerateDeviceEntries();
    /**
     * Check if there are connected device, and if so updates tooltip and overlay
     */
    void regenerateConnectedDevices();

    void sendFile();
    void browseDevices();
    void addDevice();
    void configBluetooth();
    void toggleBluetooth();
    void activeDiscoverable(bool active);

private Q_SLOTS:
    void actionTriggered();
    void browseTriggered(QString address);
    void sendTriggered(const QString &ubi);
    void UUIDsChanged(const QStringList &UUIDs);
    void poweredChanged();
    void deviceCreated(Device *device);

private:
    void onlineMode();
    void offlineMode();
    void setupDevice(Device *device);

    /**
     * Returns true or false wether there are powered adapters
     */
    bool poweredAdapters();
    void setTooltipTitleStatus(bool);
    QList<QAction*> actionsForAdapter(Adapter *adapter);
    QAction* actionForDevice(Device *device, Device *lastDevice);
private:
    QHash<QString, QString> m_supportedServices;
    QMap<void*, KAction*> m_interfaceMap;
    QList<QAction*>       m_actions;
};

Q_DECLARE_METATYPE(Monolithic::EntryInfo)

#endif // MONOLITHIC_H
