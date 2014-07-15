/*
 *    Copyright (C) 2010 Alejandro Fiestas Olivares  <alex@ufocoders.com>
 *    Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "monolithic.h"

#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QLabel>

#include <krun.h>
#include <kprocess.h>
#include <klocalizedstring.h>
#include <ktoolinvocation.h>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/LoadDeviceJob>

Monolithic::Monolithic(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::Hardware);
    setIconByName(QStringLiteral("preferences-system-bluetooth"));
    setToolTip(QStringLiteral("preferences-system-bluetooth"), QStringLiteral("Bluetooth"), QString());

    m_supportedServices.insert(i18n("Browse device"), QStringLiteral("00001106-0000-1000-8000-00805F9B34FB"));
    m_supportedServices.insert(i18n("Send Files"), QStringLiteral("00001105-0000-1000-8000-00805F9B34FB"));

    offlineMode();

    setStandardActionsEnabled(false);
    setAssociatedWidget(contextMenu());
    setStatus(KStatusNotifierItem::Active);

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    connect(m_manager, &QBluez::Manager::usableAdapterChanged, this, &Monolithic::usableAdapterChanged);

    QBluez::InitManagerJob *initJob = m_manager->init(QBluez::Manager::InitManagerAndAdapters);
    initJob->start();
    connect(initJob, &QBluez::InitManagerJob::result, [ this ](QBluez::InitManagerJob *job) {
        if (job->error()) {
            qCDebug(MONOLITHIC) << "Error initializing manager:" << job->errorText();
            return;
        }

        if (m_manager->isBluetoothOperational()) {
            onlineMode();
        }
    });
}

void Monolithic::usableAdapterChanged()
{
    if (m_manager->isBluetoothOperational()) {
        onlineMode();
    } else {
        offlineMode();
    }
}

static quint32 sortHelper(QBluez::DeviceType type)
{
    switch (type) {
        case QBluez::Any:
            return 100;
        case QBluez::Phone:
            return 0;
        case QBluez::Modem:
            return 99;
        case QBluez::Computer:
            return 1;
        case QBluez::Network:
            return 98;
        case QBluez::Headset:
            return 2;
        case QBluez::Headphones:
            return 3;
        case QBluez::OtherAudio:
            return 4;
        case QBluez::Keyboard:
            return 5;
        case QBluez::Mouse:
            return 6;
        case QBluez::Camera:
            return 7;
        case QBluez::Printer:
            return 8;
        case QBluez::Joypad:
            return 9;
        case QBluez::Tablet:
            return 10;
        default:
            break;
    }
    return 1000;
}

bool sortDevices(QBluez::Device *device1, QBluez::Device *device2)
{
    quint32 type1 = sortHelper(device1->deviceType());
    quint32 type2 = sortHelper(device2->deviceType());
    if (type1 != type2) {
        return type1 < type2;
    }
    return device1->name().compare(device2->name(), Qt::CaseInsensitive) < 0;
}

void Monolithic::regenerateDeviceEntries()
{
    QMenu *const menu = contextMenu();

    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    if (!m_manager->isBluetoothOperational()) {
        menu->addSection(i18n("Bluetooth is Off"));

        QAction *separator = new QAction(menu);
        separator->setSeparator(true);
        menu->addAction(separator);

        QAction *activeBluetooth = new QAction(i18n("Turn Bluetooth On"), menu);
        connect(activeBluetooth, SIGNAL(triggered()), this, SLOT(toggleBluetooth()));
        menu->addAction(activeBluetooth);
        return;
    }

    QAction *sendFile = new QAction(QIcon::fromTheme(QStringLiteral("edit-find-project")), i18n("Send File"), menu);
    connect(sendFile, SIGNAL(triggered()), this, SLOT(sendFile()));
    menu->addAction(sendFile);

    QAction *browseDevices = new QAction(QIcon::fromTheme(QStringLiteral("document-preview-archive")), i18n("Browse devices"), menu);
    connect(browseDevices, SIGNAL(triggered()), this, SLOT(browseDevices()));
    menu->addAction(browseDevices);

    QList<QBluez::Adapter*> adapters = m_manager->adapters();
    Q_FOREACH(QBluez::Adapter *adapter, adapters) {
        if (adapters.count() == 1) {
            menu->addSection(i18n("Known Devices"));
        } else {
            menu->addSection(adapter->name());
        }

        menu->addActions(actionsForAdapter(adapter));
    }

    menu->addSeparator();

    QAction *addDevice = new QAction(QIcon::fromTheme(QStringLiteral("edit-find-project")), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered()), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    QAction *configBluetooth = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure Bluetooth"), menu);
    connect(configBluetooth, SIGNAL(triggered(bool)), this, SLOT(configBluetooth()));
    menu->addAction(configBluetooth);

    // Shortcut configuration actions, mainly checkables for discovering and powering
    menu->addSeparator();

    QAction *discoverable = new QAction(i18n("Discoverable"), menu);
    discoverable->setCheckable(true);
    discoverable->setChecked(m_manager->usableAdapter()->isDiscoverable());
    connect(discoverable, SIGNAL(toggled(bool)), this, SLOT(activeDiscoverable(bool)));
    menu->addAction(discoverable);

    QAction *activeBluetooth = new QAction(i18n("Turn Bluetooth Off"), menu);
    connect(activeBluetooth, SIGNAL(triggered()), this, SLOT(toggleBluetooth()));
    menu->addAction(activeBluetooth);
}

void Monolithic::regenerateConnectedDevices()
{
    unsigned int connectedDevices = 0;

    if (m_manager->isBluetoothOperational()) {
        QList<QBluez::Device*> devices = m_manager->devices();
        Q_FOREACH(QBluez::Device *device, devices) {
            if (device->isConnected()) {
                ++connectedDevices;
            }
        }
    }
    if (connectedDevices > 0) {
        setOverlayIconByName(QStringLiteral("emblem-link"));
        setToolTipSubTitle(i18ncp("Number of Bluetooth connected devices", "%1 connected device", "%1 connected devices", int(connectedDevices)));
    } else if(poweredAdapters()) {
        setOverlayIconByName(QString());
        setToolTipSubTitle(QString());
    }
}

void Monolithic::onlineMode()
{
    QList<QBluez::Adapter*> adapters = m_manager->adapters();
    Q_FOREACH(QBluez::Adapter *adapter, adapters) {
        connect(adapter, &QBluez::Adapter::deviceFound, this, &Monolithic::deviceCreated);
        connect(adapter, &QBluez::Adapter::deviceRemoved, this, &Monolithic::regenerateDeviceEntries);
        connect(adapter, &QBluez::Adapter::poweredChanged, this, &Monolithic::poweredChanged);
        connect(adapter, &QBluez::Adapter::discoverableChanged, this, &Monolithic::regenerateDeviceEntries);
    }

    QList<QBluez::Device*> devices = m_manager->devices();
    Q_FOREACH(QBluez::Device *device, devices) {
        deviceCreated(device);
    }

    regenerateDeviceEntries();
    regenerateConnectedDevices();
    poweredChanged();
}

void Monolithic::actionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString service = action->data().toString();
    QBluez::Device *device = action->property("QBluez::Device").value<QBluez::Device*>();
    if (!device) {
        return;
    }

    if (service == QLatin1String("00001106-0000-1000-8000-00805F9B34FB")) {
        browseTriggered(device->address());
    } else if (service == QLatin1String("00001105-0000-1000-8000-00805F9B34FB")) {
        sendTriggered(device->ubi());
    }
}

void Monolithic::sendFile()
{
    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-sendfile"));
    process.startDetached();
}

void Monolithic::browseDevices()
{
    QUrl url(QStringLiteral("bluetooth://"));
    KRun::runUrl(url, QStringLiteral("inode/directory"), new QWidget());
}

void Monolithic::addDevice()
{
    KProcess process;
    process.setProgram(QStringLiteral("bluedevil-wizard"));
    process.startDetached();
}

void Monolithic::configBluetooth()
{
    KProcess process;
    QStringList args;
    args << QStringLiteral("bluedevildevices");
    args << QStringLiteral("bluedeviltransfer");
    args << QStringLiteral("bluedeviladapters");
    process.startDetached(QStringLiteral("kcmshell5"), args);
}

void Monolithic::toggleBluetooth()
{
    bool powered = poweredAdapters();

    QList <QBluez::Adapter*> adapters = m_manager->adapters();
    if (!adapters.isEmpty()) {
        // Invert the powered state of all adapters
        Q_FOREACH(QBluez::Adapter *adapter, adapters) {
            adapter->setPowered(!powered);
        }
    }

    // We do not call regenerateDeviceEntries because we assume that the adapter
    // proprety powered will change and then, poweredChange will be emitted
}

void Monolithic::activeDiscoverable(bool active)
{
    QList<QBluez::Adapter*> adapters = m_manager->adapters();
    if (!adapters.isEmpty()) {
        Q_FOREACH(QBluez::Adapter *adapter, adapters) {
            adapter->setDiscoverable(active);
        }
    }
}

void Monolithic::browseTriggered(QString address)
{
    QUrl url(QStringLiteral("obexftp:/"));
    url.setHost(address.replace(QLatin1Char(':'), QLatin1Char('-')));
    KRun::runUrl(url, QStringLiteral("inode/directory"), new QWidget());
}

void Monolithic::sendTriggered(const QString &UBI)
{
    KToolInvocation::kdeinitExec(QStringLiteral("bluedevil-sendfile"), QStringList() << QString("-u%1").arg(UBI));
}

void Monolithic::UUIDsChanged(const QStringList &UUIDs)
{
    Q_UNUSED(UUIDs);
    regenerateDeviceEntries();
}

void Monolithic::poweredChanged()
{
    if (!poweredAdapters()) {
        setIconByName(QStringLiteral("preferences-system-bluetooth-inactive"));
        setTooltipTitleStatus(false);
        setToolTipSubTitle(QString());
    } else {
        setIconByName(QStringLiteral("preferences-system-bluetooth"));
        setTooltipTitleStatus(true);
    }
    setOverlayIconByName(QString());
    regenerateDeviceEntries();
    regenerateConnectedDevices();
}

void Monolithic::deviceCreated(QBluez::Device *device)
{
    QBluez::LoadDeviceJob *job = device->load();
    job->start();
    connect(job, &QBluez::LoadDeviceJob::result, [ this ](QBluez::LoadDeviceJob *job) {
        if (job->error()) {
            qCDebug(MONOLITHIC) << "Error loading device" << job->errorText();
            return;
        }

        connect(job->device(), &QBluez::Device::deviceChanged, this, &Monolithic::regenerateConnectedDevices);
        connect(job->device(), &QBluez::Device::uuidsChanged, this, &Monolithic::UUIDsChanged);

        regenerateDeviceEntries();
        regenerateConnectedDevices();
    });
}

void Monolithic::offlineMode()
{
    setTooltipTitleStatus(false);

    QMenu *const menu = contextMenu();

    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    QAction *noAdaptersFound = new QAction(i18n("No adapters found"), menu);
    noAdaptersFound->setEnabled(false);
    menu->addAction(noAdaptersFound);
}

bool Monolithic::poweredAdapters()
{
    QList<QBluez::Adapter*> adapters = m_manager->adapters();

    if (adapters.isEmpty()) {
        return false;
    }

    Q_FOREACH(QBluez::Adapter *adapter, adapters) {
        if (adapter->isPowered()) {
            return true;
        }
    }

    return false;
}

void Monolithic::setTooltipTitleStatus(bool status)
{
    if (status) {
        setToolTipTitle(i18nc("When the bluetooth is enabled and powered", "Bluetooth is On"));
    } else {
        setToolTipTitle(i18nc("When the bluetooth is disabled or not powered", "Bluetooth is Off"));
    }
}

QList<QAction*> Monolithic::actionsForAdapter(QBluez::Adapter *adapter)
{
    QList<QAction*> actions;
    QList<QBluez::Device*> devices = adapter->devices();
    if (devices.isEmpty()) {
        return actions;
    }

    qStableSort(devices.begin(), devices.end(), sortDevices);
    QAction *action = 0;
    QBluez::Device *lastDevice = 0;
    Q_FOREACH (QBluez::Device *device, devices) {
        action = actionForDevice(device, lastDevice);
        actions << action;
        lastDevice = device;
    }

    m_actions << actions;

    return actions;
}

QAction* Monolithic::actionForDevice(QBluez::Device* device, QBluez::Device *lastDevice)
{
    // Create device entry
    QAction *deviceAction = new QAction(device->name(), this);
    deviceAction->setData(QVariant::fromValue<QBluez::Device*>(device));

    // We only show the icon for the first device of the type, less UI clutter
    if (!lastDevice || lastDevice->deviceType() != device->deviceType()) {
        deviceAction->setIcon(QIcon::fromTheme(device->icon()));
    }

    // Create the submenu that will hang from this device menu entry
    QMenu *const subMenu = new QMenu;
    QStringList UUIDs = device->uuids();

    QSet<QString> deviceServices = UUIDs.toSet().intersect(m_supportedServices.values().toSet());
    Q_FOREACH(QString service, deviceServices) {
        QAction *action = new QAction(m_supportedServices.key(service), subMenu);
        action->setData(service);
        action->setProperty("QBluez::Device", QVariant::fromValue<QBluez::Device*>(device));
        connect(action, &QAction::triggered, this, &Monolithic::actionTriggered);
        subMenu->addAction(action);
    }

    QAction *connectAction = new QAction(i18nc("Connect to a bluetooth device", "Connect"), deviceAction);
    connect(connectAction, &QAction::triggered, device, &QBluez::Device::connect);
    subMenu->addAction(connectAction);

//Enable when we can know if we should show Connect or not
//     if (deviceServices.isEmpty()) {
//         QAction *_unknown = new QAction(i18n("No supported services found"), deviceAction);
//         _unknown->setEnabled(false);
//         subMenu->addAction(_unknown);
//     }

    deviceAction->setMenu(subMenu);

    return deviceAction;
}

Q_LOGGING_CATEGORY(MONOLITHIC, "BlueMonolithic")
