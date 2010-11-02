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
#include "input_interface.h"

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

    offlineMode();

    if (Manager::self()->defaultAdapter()) {
        onlineMode();
    }

    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)), this, SLOT(adapterAdded()));
    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)), this, SLOT(noAdapters(Adapter*)));

    setStandardActionsEnabled(false);
    setAssociatedWidget(contextMenu());
}

void Monolithic::noAdapters(Adapter* adapter)
{
    if (!adapter) {
        offlineMode();
    }
}

void Monolithic::adapterAdded()
{
    if (status() != KStatusNotifierItem::Active) {
        onlineMode();
    }
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
    if (!Manager::self()->defaultAdapter()) {
        return;
    }

    KMenu *const menu = contextMenu();

    qDeleteAll(m_interfaceMap);
    m_interfaceMap.clear();
    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    KAction *sendFile = new KAction(KIcon("edit-find-project"), i18n("Send File"), menu);
    connect(sendFile, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(sendFile()));
    menu->addAction(sendFile);

    KAction *browseDevices = new KAction(KIcon("document-preview-archive"), i18n("Browse devices"), menu);
    connect(browseDevices, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(browseDevices()));
    menu->addAction(browseDevices);

    KAction *configBluetooth = new KAction(i18n("Configure Bluetooth"), menu);
    connect(configBluetooth, SIGNAL(triggered(bool)), this, SLOT(configBluetooth()));
    menu->addAction(configBluetooth);

    menu->addTitle(i18n("Known Devices"));
    KAction *noKnownDevices = new KAction(i18n("No known devices found"), menu);
    noKnownDevices->setEnabled(false);
    noKnownDevices->setVisible(false);
    menu->addAction(noKnownDevices);

    QList<Device*> devices = Manager::self()->defaultAdapter()->devices();
    qStableSort(devices.begin(), devices.end(), sortDevices);
    Device *lastDevice = 0;
    bool first = true;
    Q_FOREACH (Device *device, devices) {
// Create device entry
        KAction *_device = 0;
        if (!lastDevice || classToType(lastDevice->deviceClass()) != classToType(device->deviceClass())) {
            if (!first) {
                contextMenu()->addSeparator();
            } else {
                first = false;
            }
            _device = new KAction(KIcon(device->icon()), device->name(), menu);
        } else {
            _device = new KAction(device->name(), menu);
        }
        _device->setData(QVariant::fromValue<Device*>(device));

// Add this action to the own action list
        m_actions << _device;

// Create the submenu that will hang from this device menu entry
        KMenu *const _submenu = new KMenu;
        bool hasSupportedServices = false;
        QStringList UUIDs = device->UUIDs();
        EntryInfo info;
        info.device = device;
        if (UUIDs.contains("00001106-0000-1000-8000-00805F9B34FB"))  {
            KAction *_browse = new KAction(i18n("Browse device..."), _device);
            info.service = "00001106-0000-1000-8000-00805F9B34FB";
            _browse->setData(QVariant::fromValue<EntryInfo>(info));
            _submenu->addAction(_browse);
            connect(_browse, SIGNAL(triggered()), this, SLOT(browseTriggered()));
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001105-0000-1000-8000-00805F9B34FB")) {
            KAction *_send = new KAction(i18n("Send files..."), _device);
            info.service = "00001105-0000-1000-8000-00805F9B34FB";
            _send->setData(QVariant::fromValue<EntryInfo>(info));
            _submenu->addAction(_send);
            connect(_send, SIGNAL(triggered()), this, SLOT(sendTriggered()));
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001124-0000-1000-8000-00805F9B34FB")) {
            org::bluez::Input *input = new org::bluez::Input("org.bluez", device->UBI(), QDBusConnection::systemBus());
            connect(input, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
            info.service = "00001124-0000-1000-8000-00805F9B34FB";
            info.dbusService = input;
            _submenu->addTitle("Input Service");

            if (input->GetProperties().value()["Connected"].toBool()) {
                KAction *_disconnect = new KAction(i18n("Disconnect"), _device);
                m_interfaceMap[input] = _disconnect;
                _disconnect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_disconnect);
                connect(_disconnect, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
            } else {
                KAction *_connect = new KAction(i18nc("Action", "Connect"), _device);
                m_interfaceMap[input] = _connect;
                _connect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_connect);
                connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            }
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001108-0000-1000-8000-00805F9B34FB")) {
            org::bluez::Audio *audio = new org::bluez::Audio("org.bluez", device->UBI(), QDBusConnection::systemBus());
            connect(audio, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
            info.service = "00001108-0000-1000-8000-00805F9B34FB";
            info.dbusService = audio;
            _submenu->addTitle("Headset Service");

            if (audio->GetProperties().value()["State"].toString() == "connected") {
                KAction *_disconnect = new KAction(i18n("Disconnect"), _device);
                m_interfaceMap[audio] = _disconnect;
                _disconnect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_disconnect);
                connect(_disconnect, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
            } else if (audio->GetProperties().value()["State"].toString() == "connecting") {
                KAction *_connecting = new KAction(i18n("Connecting..."), _device);
                _connecting->setEnabled(false);
                m_interfaceMap[audio] = _connecting;
                _connecting->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_connecting);
            } else {
                KAction *_connect = new KAction(i18nc("Action", "Connect"), _device);
                m_interfaceMap[audio] = _connect;
                _connect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_connect);
                connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            }
            hasSupportedServices = true;
        }
        if (UUIDs.contains("0000110B-0000-1000-8000-00805F9B34FB")) {
            org::bluez::Audio *audio = new org::bluez::Audio("org.bluez", device->UBI(), QDBusConnection::systemBus());
            connect(audio, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
            info.service = "00001108-0000-1000-8000-00805F9B34FB";
            info.dbusService = audio;
            _submenu->addTitle("Audio Sink");

            if (audio->GetProperties().value()["State"].toString() == "connected") {
                KAction *_disconnect = new KAction(i18n("Disconnect"), _device);
                m_interfaceMap[audio] = _disconnect;
                _disconnect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_disconnect);
                connect(_disconnect, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
            } else if (audio->GetProperties().value()["State"].toString() == "connecting") {
                KAction *_connecting = new KAction(i18n("Connecting..."), _device);
                _connecting->setEnabled(false);
                m_interfaceMap[audio] = _connecting;
                _connecting->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_connecting);
            } else {
                KAction *_connect = new KAction(i18nc("Action", "Connect"), _device);
                m_interfaceMap[audio] = _connect;
                _connect->setData(QVariant::fromValue<EntryInfo>(info));
                _submenu->addAction(_connect);
                connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            }
            hasSupportedServices = true;
        }
        if (!hasSupportedServices) {
            KAction *_unknown = new KAction(i18n("No supported services found"), _device);
            _unknown->setEnabled(false);
            _submenu->addAction(_unknown);
        }
        _device->setMenu(_submenu);
        menu->addAction(_device);
        lastDevice = device;
    }

    if (!lastDevice) {
        noKnownDevices->setVisible(true);
    }

    QAction *separator = new QAction(menu);
    separator->setSeparator(true);
    menu->addAction(separator);

    KAction *addDevice = new KAction(KIcon("edit-find-project"), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    separator = new QAction(menu);
    separator->setSeparator(true);
    menu->addAction(separator);

    menu->addAction(KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), menu));
}

void Monolithic::onlineMode()
{
    setStatus(KStatusNotifierItem::Active);

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceCreated(Device*)), this, SLOT(deviceCreated(Device*)));
    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceDisappeared(Device*)), this, SLOT(regenerateDeviceEntries()));
    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceRemoved(Device*)), this, SLOT(regenerateDeviceEntries()));

    regenerateDeviceEntries();
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

void Monolithic::browseTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    EntryInfo entryInfo = action->data().value<EntryInfo>();

    KUrl url("obexftp:/");
    url.setHost(entryInfo.device->address().replace(':', '-'));
    KRun::runUrl(url, "inode/directory", new QWidget());
}

void Monolithic::sendTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    EntryInfo entryInfo = action->data().value<EntryInfo>();
    KToolInvocation::kdeinitExec("bluedevil-sendfile", QStringList() << QString("bluetooth://%1/").arg(entryInfo.device->address().replace(':', '-').toLower()));
}

void Monolithic::connectTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    EntryInfo entryInfo = action->data().value<EntryInfo>();
    if (entryInfo.service == "00001124-0000-1000-8000-00805F9B34FB") {
        KProcess p;
        p.setProgram("bluedevil-input", QStringList() << QString("bluetooth://%1/%2").arg(entryInfo.device->address().replace(':', '-')).arg(entryInfo.service));
        p.startDetached();
    } else if (entryInfo.service == "00001108-0000-1000-8000-00805F9B34FB") {
        KProcess p;
        p.setProgram("bluedevil-audio", QStringList() << QString("bluetooth://%1/%2").arg(entryInfo.device->address().replace(':', '-')).arg(entryInfo.service));
        p.startDetached();
    }
}

void Monolithic::disconnectTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    EntryInfo entryInfo = action->data().value<EntryInfo>();
    if (entryInfo.service == "00001124-0000-1000-8000-00805F9B34FB") {
        org::bluez::Input *input = static_cast<org::bluez::Input*>(entryInfo.dbusService);
        input->Disconnect();
    } else if (entryInfo.service == "00001108-0000-1000-8000-00805F9B34FB") {
        org::bluez::Audio *audio = static_cast<org::bluez::Audio*>(entryInfo.dbusService);
        audio->Disconnect();
    }
}

void Monolithic::propertyChanged(const QString &key, const QDBusVariant &value)
{
    KAction *action = m_interfaceMap[static_cast<void*>(sender())];

    if (!action) {
        return;
    }

    if (key == "State") {
        if (value.variant().toString() == "disconnected") {
            action->setText(i18nc("Action", "Connect"));
            action->setEnabled(true);
            disconnect(action, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
            connect(action, SIGNAL(triggered()), this, SLOT(connectTriggered()));
        } else if (value.variant().toString() == "connecting") {
            action->setText(i18n("Connecting..."));
            action->setEnabled(false);
        } else {
            action->setText(i18n("Disconnect"));
            action->setEnabled(true);
            disconnect(action, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            connect(action, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
        }
    } else if (key == "Connected") {
        if (value.variant().toBool()) {
            action->setText(i18n("Disconnect"));
            action->setEnabled(true);
            disconnect(action, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            connect(action, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
        } else {
            action->setText(i18nc("Action", "Connect"));
            action->setEnabled(true);
            disconnect(action, SIGNAL(triggered()), this, SLOT(disconnectTriggered()));
            connect(action, SIGNAL(triggered()), this, SLOT(connectTriggered()));
        }
    } else if (key == "Name") {
        action->setText(value.variant().toString());
    }
}

void Monolithic::UUIDsChanged(const QStringList &UUIDs)
{
    Device *const device = static_cast<Device*>(sender());

// Create the submenu that will hang from this device menu entry
    KMenu *const _submenu = new KMenu;
    bool hasSupportedServices = false;
    EntryInfo info;
    info.device = device;
    if (UUIDs.contains("00001106-0000-1000-8000-00805F9B34FB"))  {
        KAction *_browse = new KAction(i18n("Browse device..."), _submenu);
        info.service = "00001106-0000-1000-8000-00805F9B34FB";
        _browse->setData(QVariant::fromValue<EntryInfo>(info));
        _submenu->addAction(_browse);
        connect(_browse, SIGNAL(triggered()), this, SLOT(browseTriggered()));
        hasSupportedServices = true;
    }
    if (UUIDs.contains("00001105-0000-1000-8000-00805F9B34FB")) {
        KAction *_send = new KAction(i18n("Send files..."), _submenu);
        info.service = "00001105-0000-1000-8000-00805F9B34FB";
        _send->setData(QVariant::fromValue<EntryInfo>(info));
        _submenu->addAction(_send);
        connect(_send, SIGNAL(triggered()), this, SLOT(sendTriggered()));
        hasSupportedServices = true;
    }
    if (UUIDs.contains("00001124-0000-1000-8000-00805F9B34FB")) {
        KAction *_connect = new KAction(i18nc("Action", "Connect"), _submenu);
        org::bluez::Input *input = new org::bluez::Input("org.bluez", device->UBI(), QDBusConnection::systemBus());
        connect(input, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
        m_interfaceMap[input] = _connect;
        info.service = "00001124-0000-1000-8000-00805F9B34FB";
        info.dbusService = input;
        _connect->setData(QVariant::fromValue<EntryInfo>(info));
        _submenu->addTitle("Input Service");
        _submenu->addAction(_connect);
        connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
        hasSupportedServices = true;
    }
    if (UUIDs.contains("00001108-0000-1000-8000-00805F9B34FB")) {
        KAction *_connect = new KAction(i18nc("Action", "Connect"), _submenu);
        org::bluez::Audio *audio = new org::bluez::Audio("org.bluez", device->UBI(), QDBusConnection::systemBus());
        connect(audio, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
        m_interfaceMap[audio] = _connect;
        info.service = "00001108-0000-1000-8000-00805F9B34FB";
        info.dbusService = audio;
        _connect->setData(QVariant::fromValue<EntryInfo>(info));
        _submenu->addTitle("Headset Service");
        _submenu->addAction(_connect);
        connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
        hasSupportedServices = true;
    }
    if (UUIDs.contains("0000110B-0000-1000-8000-00805F9B34FB")) {
        KAction *_connect = new KAction(i18nc("Action", "Connect"), _submenu);
        org::bluez::Audio *audio = new org::bluez::Audio("org.bluez", device->UBI(), QDBusConnection::systemBus());
        connect(audio, SIGNAL(PropertyChanged(QString,QDBusVariant)), this, SLOT(propertyChanged(QString,QDBusVariant)));
        m_interfaceMap[audio] = _connect;
        info.service = "00001108-0000-1000-8000-00805F9B34FB";
        info.dbusService = audio;
        _connect->setData(QVariant::fromValue<EntryInfo>(info));
        _submenu->addTitle("Audio Sink");
        _submenu->addAction(_connect);
        connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
        hasSupportedServices = true;
    }
    if (!hasSupportedServices) {
        KAction *_unknown = new KAction(i18n("No supported services found"), _submenu);
        _unknown->setEnabled(false);
        _submenu->addAction(_unknown);
    }

    QAction *_device = 0;
    Q_FOREACH (QAction *action, m_actions) {
        if (action->data().value<Device*>() == device) {
            _device = action;
            break;
        }
    }
    if (_device) {
        _device->setMenu(_submenu);
    }
}

void Monolithic::deviceCreated(Device *device)
{
    connect(device, SIGNAL(UUIDsChanged(QStringList)), this, SLOT(UUIDsChanged(QStringList)));
    regenerateDeviceEntries();
}

void Monolithic::offlineMode()
{
    setStatus(KStatusNotifierItem::Passive);

    disconnect(Manager::self()->defaultAdapter(), SIGNAL(deviceCreated(Device*)), this, SLOT(deviceCreated(Device*)));
    disconnect(Manager::self()->defaultAdapter(), SIGNAL(deviceDisappeared(Device*)), this, SLOT(regenerateDeviceEntries()));
    disconnect(Manager::self()->defaultAdapter(), SIGNAL(deviceRemoved(Device*)), this, SLOT(regenerateDeviceEntries()));

    KMenu *const menu = contextMenu();

    qDeleteAll(m_interfaceMap);
    m_interfaceMap.clear();
    qDeleteAll(m_actions);
    m_actions.clear();
    qDeleteAll(menu->actions());
    menu->clear();

    KAction *sendFile = new KAction(KIcon("edit-find-project"), i18n("Send File"), menu);
    connect(sendFile, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(sendFile()));
    menu->addAction(sendFile);

    KAction *browseDevices = new KAction(KIcon("document-preview-archive"), i18n("Browse devices"), menu);
    connect(browseDevices, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(browseDevices()));
    menu->addAction(browseDevices);

    KAction *configBluetooth = new KAction(i18n("Configure Bluetooth"), menu);
    connect(configBluetooth, SIGNAL(triggered(bool)), this, SLOT(configBluetooth()));
    menu->addAction(configBluetooth);

    menu->addTitle(i18n("Known Devices"));
    KAction *noAdaptersFound = new KAction(i18n("No adapters found"), menu);
    noAdaptersFound->setEnabled(false);
    menu->addAction(noAdaptersFound);

    QAction *separator = new QAction(menu);
    separator->setSeparator(true);
    menu->addAction(separator);

    KAction *addDevice = new KAction(KIcon("edit-find-project"), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    separator = new QAction(menu);
    separator->setSeparator(true);
    menu->addAction(separator);

    menu->addAction(KStandardAction::quit(QCoreApplication::instance(), SLOT(quit()), menu));
}

Q_DECLARE_METATYPE(Device*)
