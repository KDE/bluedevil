/*
 *   SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *   SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "devicemonitor.h"
#include "bluedevil_kded.h"
#include "bluedevildaemon.h"
#include "bluedevilglobalsettings.h"

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
    , m_isParentValid(true)
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

    // Catch suspend/resume events so we can save status when suspending and
    // resume when waking up
    // It's possible that BlueDevilDaemon has been destroyed, but PrepareForSleep is
    // received before DeviceMonitor is destroyed, so a crash will happen. To prevent
    // the crash, add a check in login1PrepareForSleep to validate BlueDevilDaemon still exists.
    connect(
        parent(),
        &QObject::destroyed,
        this,
        [this] {
            m_isParentValid = false;
        },
        Qt::DirectConnection);
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(login1PrepareForSleep(bool)));
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForShutdown"),
                                         this,
                                         SLOT(login1PrepareForSleep(bool)));

    // Before setting the initial state, we need to wait for the manager to be operational
    if (m_manager->isOperational()) {
        setInitialState();
    } else {
        connect(m_manager, &BluezQt::Manager::operationalChanged, this, &DeviceMonitor::readyToSetInitialState);
    }
}

void DeviceMonitor::readyToSetInitialState(bool operational)
{
    if (operational) {
        setInitialState();
        disconnect(m_manager, &BluezQt::Manager::operationalChanged, this, &DeviceMonitor::readyToSetInitialState);
    }
}

void DeviceMonitor::setInitialState()
{
    // Set initial state
    if (GlobalSettings::launchState() == GlobalSettings::Remember) {
        restoreState();
    } else if (GlobalSettings::launchState() == GlobalSettings::Enable) {
        // Un-block Bluetooth and turn on everything
        m_manager->setBluetoothBlocked(false);
        for (BluezQt::AdapterPtr adapter : m_manager->adapters()) {
            adapter->setPowered(true);
        }
        // restoreAdapter() is scheduled to be called after 1 second when adapterAdded is emitted,
        // so we need to save state to make sure that adapter won't be turned off after restoreAdapter()
        saveState();
    } else if (GlobalSettings::launchState() == GlobalSettings::Disable) {
        // Turn off everything and block Bluetooth
        for (BluezQt::AdapterPtr adapter : m_manager->adapters()) {
            adapter->setPowered(false);
        }
        m_manager->setBluetoothBlocked(true);
        // restoreAdapter() is scheduled to be called after 1 second when adapterAdded is emitted,
        // so we need to save state to make sure that adapter won't be turned on after restoreAdapter()
        saveState();
    }
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
    if (!m_isParentValid) {
        return;
    }

    if (active) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "About to suspend";
        saveState();
    } else {
        qCDebug(BLUEDEVIL_KDED_LOG) << "About to resume";
        restoreState();
    }
}

void DeviceMonitor::saveState()
{
    KConfigGroup adaptersGroup = GlobalSettings::self()->config()->group("Adapters");

    if (m_manager->isBluetoothBlocked()) {
        GlobalSettings::setBluetoothBlocked(true);
    } else {
        GlobalSettings::setBluetoothBlocked(false);

        // Save powered state for each adapter
        Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
            const QString key = QStringLiteral("%1_powered").arg(adapter->address());
            adaptersGroup.writeEntry<bool>(key, adapter->isPowered());
        }
    }

    QStringList connectedDevices;

    Q_FOREACH (BluezQt::DevicePtr device, m_manager->devices()) {
        if (device->isConnected()) {
            connectedDevices.append(device->address());
        }
    }

    GlobalSettings::setConnectedDevices(connectedDevices);

    GlobalSettings::self()->save();
}

void DeviceMonitor::restoreState()
{
    KConfigGroup adaptersGroup = GlobalSettings::self()->config()->group("Adapters");

    // Restore blocked/unblocked state
    m_manager->setBluetoothBlocked(GlobalSettings::bluetoothBlocked());

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        const QString key = QStringLiteral("%1_powered").arg(adapter->address());
        adapter->setPowered(adaptersGroup.readEntry<bool>(key, true));
    }

    const QStringList &connectedDevices = GlobalSettings::connectedDevices();

    for (const QString &addr : connectedDevices) {
        BluezQt::DevicePtr device = m_manager->deviceForAddress(addr);
        if (device) {
            device->connectToDevice();
        }
    }
}

void DeviceMonitor::restoreAdapter(BluezQt::AdapterPtr adapter)
{
    KConfigGroup adaptersGroup = GlobalSettings::self()->config()->group("Adapters");

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
            qCDebug(BLUEDEVIL_KDED_LOG) << "Adding place" << url;
            QString icon = device->icon();
            if (icon == QLatin1String("phone")) {
                icon.prepend(QLatin1String("smart")); // Better breeze icon
            }
            places()->addPlace(device->name(), url, icon);
        }
    } else {
        if (places()->url(index) == url) {
            qCDebug(BLUEDEVIL_KDED_LOG) << "Removing place" << url;
            places()->removePlace(index);
        }
    }
}

#include "moc_devicemonitor.cpp"
