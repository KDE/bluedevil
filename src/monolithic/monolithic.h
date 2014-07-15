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

#include <QLoggingCategory>

#include <kstatusnotifieritem.h>

namespace QBluez {
    class Manager;
    class Adapter;
    class Device;
}

class QAction;

class Monolithic : public KStatusNotifierItem
{
    Q_OBJECT

public:
    Monolithic(QObject* parent = 0);

public Q_SLOTS:
    void usableAdapterChanged();

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
    void deviceCreated(QBluez::Device *device);

private:
    void onlineMode();
    void offlineMode();

    /**
     * Returns true or false wether there are powered adapters
     */
    bool poweredAdapters();
    void setTooltipTitleStatus(bool);
    QList<QAction*> actionsForAdapter(QBluez::Adapter *adapter);
    QAction *actionForDevice(QBluez::Device *device, QBluez::Device *lastDevice);

private:
    QBluez::Manager *m_manager;
    QHash<QString, QString> m_supportedServices;
    QList<QAction*> m_actions;
};

Q_DECLARE_LOGGING_CATEGORY(MONOLITHIC)

#endif // MONOLITHIC_H
