/*
 *   SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "devicemonitor.h"
#include "bluedevildaemon.h"
#include "debug_p.h"

#include <QTimer>

#include <KConfigGroup>
#include <KDirNotify>
#include <KFilePlacesModel>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/Manager>
#include <BluezQt/Services>

DeviceMonitor::DeviceMonitor(BlueDevilDaemon *daemon)
    : QObject(daemon)
    , m_manager(daemon->manager())
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
                                         SLOT(login1PrepareForSleep(bool)));

    restoreState();
}

KFilePlacesModel *DeviceMonitor::places()
{
    if (!m_places) {
        m_places = new KFilePlacesModel(this);
    }

    return m_places;
}

void DeviceMonitor::bluetoothOperationalChanged(bool operational)
{
    if (!operational) {
        clearPlaces();
    }
}

void DeviceMonitor::adapterAdded(BluezQt::AdapterPtr adapter)
{
    // Workaround bluez-qt not registering the powered change after resume from suspend
    QTimer::singleShot(1000, this, [this, adapter]() {
        restoreAdapter(adapter);
    });
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
    Q_ASSERT(qobject_cast<BluezQt::Device *>(sender()));

    BluezQt::DevicePtr device = static_cast<BluezQt::Device *>(sender())->toSharedPtr();
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
            device->connectToDevice();
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
    for (int i = 0; i < places()->rowCount(); ++i) {
        const QModelIndex &index = places()->index(i, 0);
        if (places()->url(index).scheme() == QLatin1String("obexftp")) {
            places()->removePlace(index);
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

    const QModelIndex &index = places()->closestItem(url);

    if (device->isConnected()) {
        if (places()->url(index) != url) {
            qCDebug(BLUEDAEMON) << "Adding place" << url;
            QString icon = device->icon();
            if (icon == QLatin1String("phone")) {
                icon.prepend(QLatin1String("smart")); // Better breeze icon
            }
            places()->addPlace(device->name(), url, icon);
        }
    } else {
        if (places()->url(index) == url) {
            qCDebug(BLUEDAEMON) << "Removing place" << url;
            places()->removePlace(index);
        }
    }
}
