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
#include "audio_interface.h"

#include <KDebug>
#include <kmenu.h>
#include <kaction.h>
#include <kprocess.h>
#include <ktoolinvocation.h>
#include <klocalizedstring.h>
#include <krun.h>

#include <bluedevil/bluedevil.h>
#include <kactioncollection.h>

using namespace BlueDevil;

Monolithic::Monolithic(QObject* parent)
    : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::Hardware);
    setIconByName("preferences-system-bluetooth");
    setToolTip("preferences-system-bluetooth", "Bluetooth", "");

    m_supportedServices.insert(i18n("Browse device"), "00001106-0000-1000-8000-00805F9B34FB");
    m_supportedServices.insert(i18n("Send Files"), "00001105-0000-1000-8000-00805F9B34FB");

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
    KMenu *const menu = contextMenu();

    qDeleteAll(m_interfaceMap);
    m_interfaceMap.clear();
    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    //If there are adapters (because we're in this function) but any of them is powered
    if (!poweredAdapters()) {
        menu->addTitle(i18n("Bluetooth is Off"));

        QAction *separator = new QAction(menu);
        separator->setSeparator(true);
        menu->addAction(separator);

        KAction *activeBluetooth = new KAction(i18n("Turn Bluetooth On"), menu);
        connect(activeBluetooth, SIGNAL(triggered()), this, SLOT(toggleBluetooth()));
        menu->addAction(activeBluetooth);
        return;
    }

    KAction *sendFile = new KAction(KIcon("edit-find-project"), i18n("Send File"), menu);
    connect(sendFile, SIGNAL(triggered()), this, SLOT(sendFile()));
    menu->addAction(sendFile);

    KAction *browseDevices = new KAction(KIcon("document-preview-archive"), i18n("Browse devices"), menu);
    connect(browseDevices, SIGNAL(triggered()), this, SLOT(browseDevices()));
    menu->addAction(browseDevices);

    QList<Adapter*> adapters = Manager::self()->adapters();
    Q_FOREACH(Adapter* adapter, adapters) {
        if (adapters.count() == 1) {
            menu->addTitle(i18n("Known Devices"));
        } else {
            menu->addTitle(adapter->name());
        }

        menu->addActions(actionsForAdapter(adapter));
    }

    menu->addSeparator();

    KAction *addDevice = new KAction(KIcon("edit-find-project"), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered()), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    KAction *configBluetooth = new KAction(KIcon("configure"), i18n("Configure Bluetooth"), menu);
    connect(configBluetooth, SIGNAL(triggered(bool)), this, SLOT(configBluetooth()));
    menu->addAction(configBluetooth);

//Shortcut configuration actions, mainly checkables for discovering and powering
    menu->addSeparator();

    Adapter *usableAdapter = Manager::self()->usableAdapter();

    KAction *discoverable = new KAction(i18n("Discoverable"), menu);
    discoverable->setCheckable(true);
    discoverable->setChecked(usableAdapter && usableAdapter->isDiscoverable());
    connect(discoverable, SIGNAL(toggled(bool)), this, SLOT(activeDiscoverable(bool)));
    menu->addAction(discoverable);

    KAction *activeBluetooth = new KAction(i18n("Turn Bluetooth Off"), menu);
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
        setOverlayIconByName("emblem-link");
        setToolTipSubTitle(i18ncp("Number of Bluetooth connected devices", "%1 connected device", "%1 connected devices", int(connectedDevices)));
    } else if(poweredAdapters()) {
        setOverlayIconByName(QString());
        setToolTipSubTitle("");
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
    KAction *action = qobject_cast<KAction*>(sender());
    QString service = action->data().toString();
    Device *device = Manager::self()->deviceForUBI(action->property("UBI").toString());
    if (!device) {
        return;
    }

    if (service == "00001106-0000-1000-8000-00805F9B34FB") {
        browseTriggered(device->address());
    } else if (service == "00001105-0000-1000-8000-00805F9B34FB") {
        sendTriggered(device->UBI());
    }
}

void Monolithic::sendFile()
{
    KProcess process;
    process.setProgram("bluedevil-sendfile");
    process.startDetached();
}

void Monolithic::browseDevices()
{
    KUrl url("bluetooth://");
    KRun::runUrl(url, "inode/directory", new QWidget());
}

void Monolithic::addDevice()
{
    KProcess process;
    process.setProgram("bluedevil-wizard");
    process.startDetached();
}

void Monolithic::configBluetooth()
{
    KProcess process;
    QStringList args;
    args << "bluedevildevices";
    args << "bluedeviltransfer";
    args << "bluedeviladapters";
    process.startDetached("kcmshell4", args);
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
    KUrl url("obexftp:/");
    url.setHost(address.replace(':', '-'));
    KRun::runUrl(url, "inode/directory", new QWidget());
}

void Monolithic::sendTriggered(const QString &UBI)
{
    KToolInvocation::kdeinitExec("bluedevil-sendfile", QStringList() << QString("-u%1").arg(UBI));
}

void Monolithic::UUIDsChanged(const QStringList &UUIDs)
{
    Q_UNUSED(UUIDs);
    regenerateDeviceEntries();
}

void Monolithic::poweredChanged()
{
    if (!poweredAdapters()) {
        setIconByName("preferences-system-bluetooth-inactive");
        setTooltipTitleStatus(false);
        setToolTipSubTitle("");
    } else {
        setIconByName("preferences-system-bluetooth");
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

    KMenu *const menu = contextMenu();

    qDeleteAll(m_interfaceMap);
    m_interfaceMap.clear();
    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    KAction *noAdaptersFound = new KAction(i18n("No adapters found"), menu);
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
    KAction *deviceAction = new KAction(device->name(), this);
    deviceAction->setData(QVariant::fromValue<Device*>(device));

    //We only show the icon for the first device of the type, less UI clutter
    if (!lastDevice || classToType(lastDevice->deviceClass()) != classToType(device->deviceClass())) {
        deviceAction->setIcon(KIcon(device->icon()));
    }

    // Create the submenu that will hang from this device menu entry
    KMenu *const subMenu = new KMenu;
    QStringList UUIDs = device->UUIDs();

    QSet <QString> deviceServices = UUIDs.toSet().intersect(m_supportedServices.values().toSet());
    Q_FOREACH(QString service, deviceServices) {
        KAction *action = new KAction(m_supportedServices.key(service), subMenu);
        action->setData(service);
        action->setProperty("UBI", device->UBI());
        connect(action, SIGNAL(triggered()), SLOT(actionTriggered()));
        subMenu->addAction(action);
    }

    if (device->isConnected()) {
        KAction *reconnectAction = new KAction(i18nc("Re-connect to a bluetooth device", "Re-connect"), deviceAction);
        KAction* disconnectAction = new KAction(i18nc("Disconnect to a bluetooth device", "Disconnect"), deviceAction);
        connect(reconnectAction, SIGNAL(triggered()), device, SLOT(connectDevice()));
        connect(disconnectAction, SIGNAL(triggered()), device, SLOT(disconnect()));
        subMenu->addAction(reconnectAction);
        subMenu->addAction(disconnectAction);
    } else {
        KAction *connectAction = new KAction(i18nc("Connect to a bluetooth device", "Connect"), deviceAction);
        connect(connectAction, SIGNAL(triggered()), device, SLOT(connectDevice()));
        subMenu->addAction(connectAction);
    }

//Enable when we can know if we should show Connect or not
//     if (deviceServices.isEmpty()) {
//         KAction *_unknown = new KAction(i18n("No supported services found"), deviceAction);
//         _unknown->setEnabled(false);
//         subMenu->addAction(_unknown);
//     }

    deviceAction->setMenu(subMenu);

    return deviceAction;
}

Q_DECLARE_METATYPE(Device*)
