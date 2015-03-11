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

#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QLabel>

#include <krun.h>
#include <kprocess.h>
#include <klocalizedstring.h>
#include <ktoolinvocation.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

Monolithic::Monolithic(QObject* parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::Hardware);
    setIconByName(QStringLiteral("preferences-system-bluetooth"));
    setToolTip(QStringLiteral("preferences-system-bluetooth"), QStringLiteral("Bluetooth"), QString());

    m_supportedServices.insert(i18n("Browse device"), QStringLiteral("00001106-0000-1000-8000-00805F9B34FB"));
    m_supportedServices.insert(i18n("Send Files"), QStringLiteral("00001105-0000-1000-8000-00805F9B34FB"));

    offlineMode();

    if (Manager::self()->usableAdapter()) {
        onlineMode();
    }

    connect(Manager::self(), SIGNAL(adapterRemoved(Adapter*)), this, SLOT(adapterChanged()));
    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)), this, SLOT(adapterChanged()));
    connect(Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)), this, SLOT(adapterChanged()));

    setStandardActionsEnabled(false);
    setAssociatedWidget(contextMenu());

    poweredChanged();
}

void Monolithic::adapterChanged()
{
    offlineMode();

    if (Manager::self()->usableAdapter()) {
        onlineMode();
    }

    poweredChanged();
}

quint32 sortHelper(quint32 type)
{
    switch (type) {
        case BLUETOOTH_TYPE_ANY:
            return 100;
        case BLUETOOTH_TYPE_PHONE:
            return 0;
        case BLUETOOTH_TYPE_MODEM:
            return 99;
        case BLUETOOTH_TYPE_COMPUTER:
            return 1;
        case BLUETOOTH_TYPE_NETWORK:
            return 98;
        case BLUETOOTH_TYPE_HEADSET:
            return 2;
        case BLUETOOTH_TYPE_HEADPHONES:
            return 3;
        case BLUETOOTH_TYPE_OTHER_AUDIO:
            return 4;
        case BLUETOOTH_TYPE_KEYBOARD:
            return 5;
        case BLUETOOTH_TYPE_MOUSE:
            return 6;
        case BLUETOOTH_TYPE_CAMERA:
            return 7;
        case BLUETOOTH_TYPE_PRINTER:
            return 8;
        case BLUETOOTH_TYPE_JOYPAD:
            return 9;
        case BLUETOOTH_TYPE_TABLET:
            return 10;
        default:
            break;
    }
    return 1000;
}

bool sortDevices(Device *device1, Device *device2)
{
    quint32 type1 = sortHelper(classToType(device1->deviceClass()));
    quint32 type2 = sortHelper(classToType(device2->deviceClass()));
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

    //If there are adapters (because we're in this function) but any of them is powered
    if (!poweredAdapters()) {
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

    QList<Adapter*> adapters = Manager::self()->adapters();
    Q_FOREACH(Adapter* adapter, adapters) {
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

//Shortcut configuration actions, mainly checkables for discovering and powering
    menu->addSeparator();

    Adapter *usableAdapter = Manager::self()->usableAdapter();

    QAction *discoverable = new QAction(i18n("Discoverable"), menu);
    discoverable->setCheckable(true);
    discoverable->setChecked(usableAdapter && usableAdapter->isDiscoverable());
    connect(discoverable, SIGNAL(toggled(bool)), this, SLOT(activeDiscoverable(bool)));
    menu->addAction(discoverable);

    QAction *activeBluetooth = new QAction(i18n("Turn Bluetooth Off"), menu);
    connect(activeBluetooth, SIGNAL(triggered()), this, SLOT(toggleBluetooth()));
    menu->addAction(activeBluetooth);

    menu->addSeparator();

//     menu->addAction(KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), menu));
}

void Monolithic::regenerateConnectedDevices()
{
    unsigned int connectedDevices = 0;
    if (Manager::self()->usableAdapter()) {
        QList<Device*> devices = Manager::self()->devices();
        Q_FOREACH(Device* device, devices) {
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

void Monolithic::setupDevice(Device *device)
{
    connect(device, SIGNAL(propertyChanged(QString,QVariant)), this, SLOT(regenerateConnectedDevices()));
    connect(device, SIGNAL(connectedChanged(bool)), this, SLOT(regenerateDeviceEntries()));
    connect(device, SIGNAL(UUIDsChanged(QStringList)), this, SLOT(UUIDsChanged(QStringList)));
}

void Monolithic::onlineMode()
{
    setStatus(Active);

    QList<Adapter*> adapters = Manager::self()->adapters();
    Q_FOREACH(Adapter *adapter, adapters) {
        connect(adapter, SIGNAL(deviceFound(Device*)), SLOT(deviceCreated(Device*)));
        connect(adapter, SIGNAL(deviceRemoved(Device*)), SLOT(regenerateDeviceEntries()));
        connect(adapter, SIGNAL(poweredChanged(bool)), SLOT(poweredChanged()));
        connect(adapter, SIGNAL(discoverableChanged(bool)), SLOT(regenerateDeviceEntries()));
    }

    QList<Device*> devices = Manager::self()->devices();
    Q_FOREACH(Device* device, devices) {
        setupDevice(device);
    }
}

void Monolithic::actionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString service = action->data().toString();
    Device *device = Manager::self()->deviceForUBI(action->property("UBI").toString());
    if (!device) {
        return;
    }

    if (service == QLatin1String("00001106-0000-1000-8000-00805F9B34FB")) {
        browseTriggered(device->address());
    } else if (service == QLatin1String("00001105-0000-1000-8000-00805F9B34FB")) {
        sendTriggered(device->UBI());
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
    args << QLatin1String("bluedevildevices");
    args << QLatin1String("bluedeviltransfer");
    args << QLatin1String("bluedeviladapters");
    process.startDetached(QStringLiteral("kcmshell5"), args);
}

void Monolithic::toggleBluetooth()
{
    bool powered = false;
    if (poweredAdapters()) {
        powered = true;
    }

    QList <Adapter*> adapters = Manager::self()->adapters();
    if (!adapters.isEmpty()) {
        Q_FOREACH(Adapter *adapter, adapters) {
            adapter->setPowered(!powered);//If there were powered devices, unpower them.
        }
    }

    //We do not call regenerateDeviceEntries because we assume that the adapter proprety powered will change
    //and then, poweredChange will be emitted
}

void Monolithic::activeDiscoverable(bool active)
{
    QList <Adapter*> adapters = Manager::self()->adapters();
    if (!adapters.isEmpty()) {
        Q_FOREACH(Adapter *adapter, adapters) {
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
        setToolTipSubTitle("");
    } else {
        setIconByName(QStringLiteral("preferences-system-bluetooth"));
        setTooltipTitleStatus(true);
    }
    setOverlayIconByName(QString());
    regenerateDeviceEntries();
    regenerateConnectedDevices();
}

void Monolithic::deviceCreated(Device *device)
{
    setupDevice(device);
    regenerateDeviceEntries();
    regenerateConnectedDevices();
}

void Monolithic::offlineMode()
{
    setStatus(Passive);
    setTooltipTitleStatus(false);

    QMenu *const menu = contextMenu();

    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    QAction *noAdaptersFound = new QAction(i18n("No adapters found"), menu);
    noAdaptersFound->setEnabled(false);
    menu->addAction(noAdaptersFound);

    QAction *separator = new QAction(menu);
    separator->setSeparator(true);
    menu->addAction(separator);
//     menu->addAction(KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), menu));

    QList<Adapter*> adapters = Manager::self()->adapters();
    Q_FOREACH(Adapter *adapter, adapters) {
        adapter->disconnect(this);
    }

    QList<Device*> devices = Manager::self()->devices();
    Q_FOREACH(Device* device, devices) {
        device->QObject::disconnect(this);
    }
}

bool Monolithic::poweredAdapters()
{
    QList <Adapter*> adapters = Manager::self()->adapters();

    if (adapters.isEmpty()) {
        return false;
    }

    Q_FOREACH(Adapter* adapter, adapters) {
        if (adapter->isPowered()) {
            return true;
        }
    }

    return false;
}

void Monolithic::setTooltipTitleStatus(bool status)
{
    if (status) {
        setToolTipTitle(i18nc("When the bluetooth is enabled and powered","Bluetooth is On"));
    } else {
        setToolTipTitle(i18nc("When the bluetooth is disabled or not powered","Bluetooth is Off"));
    }
}

QList< QAction* > Monolithic::actionsForAdapter(Adapter* adapter)
{
    QList<QAction*> actions;
    QList<Device*> devices = adapter->devices();
    if (devices.isEmpty()) {
        return actions;
    }

    qStableSort(devices.begin(), devices.end(), sortDevices);
    QAction *action = 0;
    Device *lastDevice = 0;
    Q_FOREACH (Device *device, devices) {
        action = actionForDevice(device, lastDevice);
        actions << action;
        lastDevice = device;
    }

    m_actions << actions;

    return actions;
}

QAction* Monolithic::actionForDevice(Device* device, Device *lastDevice)
{
    // Create device entry
    QAction *deviceAction = new QAction(device->name(), this);
    deviceAction->setData(QVariant::fromValue<Device*>(device));

    //We only show the icon for the first device of the type, less UI clutter
    if (!lastDevice || classToType(lastDevice->deviceClass()) != classToType(device->deviceClass())) {
        deviceAction->setIcon(QIcon::fromTheme(device->icon()));
    }

    // Create the submenu that will hang from this device menu entry
    QMenu *const subMenu = new QMenu;
    QStringList UUIDs = device->UUIDs();

    QSet <QString> deviceServices = UUIDs.toSet().intersect(m_supportedServices.values().toSet());
    Q_FOREACH(QString service, deviceServices) {
        QAction *action = new QAction(m_supportedServices.key(service), subMenu);
        action->setData(service);
        action->setProperty("UBI", device->UBI());
        connect(action, SIGNAL(triggered()), SLOT(actionTriggered()));
        subMenu->addAction(action);
    }

    if (device->isConnected()) {
        QAction *reconnectAction = new QAction(i18nc("Re-connect to a bluetooth device", "Re-connect"), deviceAction);
        QAction* disconnectAction = new QAction(i18nc("Disconnect to a bluetooth device", "Disconnect"), deviceAction);
        connect(reconnectAction, SIGNAL(triggered()), device, SLOT(connectDevice()));
        connect(disconnectAction, SIGNAL(triggered()), device, SLOT(disconnect()));
        subMenu->addAction(reconnectAction);
        subMenu->addAction(disconnectAction);
    } else {
        QAction *connectAction = new QAction(i18nc("Connect to a bluetooth device", "Connect"), deviceAction);
        connect(connectAction, SIGNAL(triggered()), device, SLOT(connectDevice()));
        subMenu->addAction(connectAction);
    }

//Enable when we can know if we should show Connect or not
//     if (deviceServices.isEmpty()) {
//         QAction *_unknown = new QAction(i18n("No supported services found"), deviceAction);
//         _unknown->setEnabled(false);
//         subMenu->addAction(_unknown);
//     }

    deviceAction->setMenu(subMenu);

    return deviceAction;
}

Q_DECLARE_METATYPE(Device*)
