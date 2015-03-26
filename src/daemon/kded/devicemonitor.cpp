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

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/Services>

DeviceMonitor::DeviceMonitor(BluezQt::Manager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
    , m_places(new KFilePlacesModel(this))
    , m_config(KSharedConfig::openConfig(QStringLiteral("bluedevilglobalrc")))
{
    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        adapterAdded(adapter);
    }

    Q_FOREACH (BluezQt::DevicePtr device, m_manager->devices()) {
        deviceAdded(device);
    }

    connect(m_manager, &BluezQt::Manager::adapterAdded, this, &DeviceMonitor::adapterAdded);
    connect(m_manager, &BluezQt::Manager::deviceAdded, this, &DeviceMonitor::deviceAdded);
    connect(m_manager, &BluezQt::Manager::bluetoothOperationalChanged, this, &DeviceMonitor::bluetoothOperationalChanged);

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

void DeviceMonitor::bluetoothOperationalChanged(bool operational)
{
    if (!operational) {
        clearPlaces();
    }
}

void DeviceMonitor::adapterAdded(BluezQt::AdapterPtr adapter)
{
    restoreAdapter(adapter);
}

void DeviceMonitor::deviceAdded(BluezQt::DevicePtr device)
{
    updateDevicePlace(device);
    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("bluetooth:/")));

    connect(device.data(), &BluezQt::Device::connectedChanged, this, &DeviceMonitor::deviceConnectedChanged);
}

void DeviceMonitor::deviceConnectedChanged(bool connected)
{
    Q_UNUSED(connected)
    Q_ASSERT(qobject_cast<BluezQt::Device*>(sender()));

    BluezQt::DevicePtr device = static_cast<BluezQt::Device*>(sender())->toSharedPtr();
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

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        const QString key = QStringLiteral("%1_powered").arg(adapter->address());
        adaptersGroup.writeEntry<bool>(key, adapter->isPowered());
    }

    QStringList connectedDevices;

    Q_FOREACH (BluezQt::DevicePtr device, m_manager->devices()) {
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

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        const QString key = QStringLiteral("%1_powered").arg(adapter->address());
        adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
    }

    KConfigGroup devicesGroup = m_config->group("Devices");
    const QStringList &connectedDevices = devicesGroup.readEntry<QStringList>(QStringLiteral("connectedDevices"), QStringList());

    Q_FOREACH (const QString &addr, connectedDevices) {
        BluezQt::DevicePtr device = m_manager->deviceForAddress(addr);
        if (device) {
            device->connectDevice();
        }
    }
}

void DeviceMonitor::restoreAdapter(BluezQt::AdapterPtr adapter)
{
    KConfigGroup adaptersGroup = m_config->group("Adapters");

    const QString &key = QStringLiteral("%1_powered").arg(adapter->address());
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

void DeviceMonitor::updateDevicePlace(BluezQt::DevicePtr device)
{
    if (!device->uuids().contains(BluezQt::Services::ObexFileTransfer)) {
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
