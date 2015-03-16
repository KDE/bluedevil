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

#include "devicemonitor.h"
#include "debug_p.h"

#include <KDirNotify>
#include <KConfigGroup>
#include <KFilePlacesModel>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

DeviceMonitor::DeviceMonitor(QObject *parent)
    : QObject(parent)
    , m_manager(Manager::self())
    , m_places(new KFilePlacesModel(this))
    , m_config(KSharedConfig::openConfig(QStringLiteral("bluedevilglobalrc")))
{
    Q_FOREACH (Adapter *adapter, m_manager->adapters()) {
        adapterAdded(adapter);
    }

    Q_FOREACH (Device *device, m_manager->devices()) {
        deviceAdded(device);
    }

    connect(m_manager, &Manager::adapterAdded, this, &DeviceMonitor::adapterAdded);
    connect(m_manager, &Manager::usableAdapterChanged, this, &DeviceMonitor::usableAdapterChanged);

    // Catch suspend/resume events
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(login1PrepareForSleep(bool))
                                         );

    restoreState();
}

DeviceMonitor::~DeviceMonitor()
{
    saveState();
}

void DeviceMonitor::usableAdapterChanged(Adapter *adapter)
{
    if (!adapter) {
        clearPlaces();
    }
}

void DeviceMonitor::adapterAdded(Adapter *adapter)
{
    restoreAdapter(adapter);

    connect(adapter, &Adapter::deviceFound, this, &DeviceMonitor::deviceAdded);
}

void DeviceMonitor::deviceAdded(Device *device)
{
    updateDevicePlace(device);
    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("bluetooth:/")));

    connect(device, &Device::connectedChanged, this, &DeviceMonitor::deviceConnectedChanged);
}

void DeviceMonitor::deviceConnectedChanged(bool connected)
{
    Q_UNUSED(connected)
    Q_ASSERT(qobject_cast<Device*>(sender()));

    Device *device = static_cast<Device*>(sender());
    updateDevicePlace(device);
}

void DeviceMonitor::login1PrepareForSleep(bool active)
{
    if (active) {
        qCDebug(BLUEDAEMON) << "About to suspend";
        saveState();
    } else {
        qCDebug(BLUEDAEMON) << "About to resume";
        restoreState();
    }
}

void DeviceMonitor::saveState()
{
    KConfigGroup adaptersGroup = m_config->group("Adapters");

    Q_FOREACH (Adapter *adapter, m_manager->adapters()) {
        const QString key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
        adaptersGroup.writeEntry<bool>(key, adapter->isPowered());
    }

    QStringList connectedDevices;

    Q_FOREACH (Device *device, m_manager->devices()) {
        if (device->isConnected()) {
            connectedDevices.append(device->address());
        }
    }

    KConfigGroup devicesGroup = m_config->group("Devices");
    devicesGroup.writeEntry<QStringList>(QStringLiteral("connectedDevices"), connectedDevices);

    m_config->sync();
}

void DeviceMonitor::restoreState()
{
    KConfigGroup adaptersGroup = m_config->group("Adapters");

    Q_FOREACH (Adapter *adapter, m_manager->adapters()) {
        const QString key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
        adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
    }

    KConfigGroup devicesGroup = m_config->group("Devices");
    const QStringList &connectedDevices = devicesGroup.readEntry<QStringList>(QStringLiteral("connectedDevices"), QStringList());

    Q_FOREACH (const QString &addr, connectedDevices) {
        Device *device = deviceForAddress(addr);
        if (device) {
            device->connectDevice();
        }
    }
}

void DeviceMonitor::restoreAdapter(Adapter *adapter)
{
    KConfigGroup adaptersGroup = m_config->group("Adapters");

    const QString &key = QString(QStringLiteral("%1_powered")).arg(adapter->address());
    adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
}

void DeviceMonitor::clearPlaces()
{
    for (int i = 0; i < m_places->rowCount(); ++i) {
        const QModelIndex &index = m_places->index(i, 0);
        if (m_places->url(index).scheme() == QLatin1String("obexftp")) {
            m_places->removePlace(index);
            i--;
        }
    }
}

void DeviceMonitor::updateDevicePlace(Device *device)
{
    // OBEX File Transfer Protocol
    if (!device->UUIDs().contains(QStringLiteral("00001106-0000-1000-8000-00805F9B34FB"))) {
        return;
    }

    QUrl url;
    url.setScheme(QStringLiteral("obexftp"));
    url.setHost(device->address().replace(QLatin1Char(':'), QLatin1Char('-')));

    const QModelIndex &index = m_places->closestItem(url);

    if (device->isConnected()) {
        if (m_places->url(index) != url) {
            qCDebug(BLUEDAEMON) << "Adding place" << url;
            QString icon = device->icon();
            if (icon == QLatin1String("phone")) {
                icon.prepend(QLatin1String("smart")); // Better breeze icon
            }
            m_places->addPlace(device->name(), url, icon);
        }
    } else {
        if (m_places->url(index) == url) {
            qCDebug(BLUEDAEMON) << "Removing place" << url;
            m_places->removePlace(index);
        }
    }
}

Device *DeviceMonitor::deviceForAddress(const QString &address) const
{
    Device *device = 0;

    Q_FOREACH (Adapter *adapter, m_manager->adapters()) {
        if (!adapter->isPowered()) {
            continue;
        }

        device = adapter->deviceForAddress(address);
        if (device) {
            break;
        }
    }

    return device;
}
