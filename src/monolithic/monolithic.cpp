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
#include "headset_interface.h"
#include "input_interface.h"

#include <kmenu.h>
#include <kaction.h>
#include <kprocess.h>
#include <ktoolinvocation.h>
#include <klocalizedstring.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

Monolithic::Monolithic(QObject* parent): KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::Hardware);
    setIconByName("preferences-system-bluetooth");

    onlineMode();
    if (!Manager::self()->defaultAdapter()) {
        offlineMode();
    }

    connect(Manager::self(), SIGNAL(adapterAdded(Adapter*)), this,
        SLOT(adapterAdded()));
    connect(Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)), this,
        SLOT(noAdapters(Adapter*)));
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

void Monolithic::generateDeviceEntries()
{
    if (!Manager::self()->defaultAdapter()) {
        return;
    }

    KMenu *const menu = contextMenu();

    menu->addTitle(i18n("Known Devices"));

    QList<Device*> devices = Manager::self()->defaultAdapter()->devices();
    qStableSort(devices.begin(), devices.end(), sortDevices);
    Device *lastDevice = 0;
    bool first = true;
    Q_FOREACH (Device *device, devices) {
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
        KMenu *const _submenu = new KMenu;
        bool hasSupportedServices = false;
        QStringList UUIDs = device->UUIDs();
        if (UUIDs.contains("00001106-0000-1000-8000-00805f9b34fb"))  {
            KAction *_browse = new KAction(i18n("Browse device..."), _device);
            _browse->setData(QVariant::fromValue<Device*>(device));
            _submenu->addAction(_browse);
            connect(_browse, SIGNAL(triggered()), this, SLOT(browseTriggered()));
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001105-0000-1000-8000-00805f9b34fb")) {
            KAction *_send = new KAction(i18n("Send files..."), _device);
            _send->setData(QVariant::fromValue<Device*>(device));
            _submenu->addAction(_send);
            connect(_send, SIGNAL(triggered()), this, SLOT(sendTriggered()));
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001124-0000-1000-8000-00805f9b34fb")) {
            KAction *_connect = new KAction(i18n("Connect"), _device);
            _connect->setData(QVariant::fromValue<Device*>(device));
            _submenu->addTitle("Input Service");
            _submenu->addAction(_connect);
            connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            hasSupportedServices = true;
        }
        if (UUIDs.contains("00001108-0000-1000-8000-00805f9b34fb")) {
            KAction *_connect = new KAction(i18n("Connect"), _device);
            _connect->setData(QVariant::fromValue<Device*>(device));
            _submenu->addTitle("Headset Service");
            _submenu->addAction(_connect);
            connect(_connect, SIGNAL(triggered()), this, SLOT(connectTriggered()));
            hasSupportedServices = true;
        }
        if (hasSupportedServices) {
            _device->setData(QVariant::fromValue<Device*>(device));
        } else {
            KAction *_unknown = new KAction(i18n("Not supported services found"), _device);
            _unknown->setEnabled(false);
            _submenu->addAction(_unknown);
        }
        _device->setMenu(_submenu);
        menu->addAction(_device);
        lastDevice = device;
    }
}

void Monolithic::onlineMode()
{
    setStatus(KStatusNotifierItem::Active);

    KMenu *menu = contextMenu();

    KAction *sendFile = new KAction(KIcon("edit-find-project"), i18n("Send File"), menu);
    connect(sendFile, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(sendFile()));
    menu->addAction(sendFile);

    KAction *configReceive = new KAction(KIcon("folder-tar"),i18n("Receive files configuration"), menu);
    connect(configReceive, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(configReceive()));
    menu->addAction(configReceive);

    KAction *deviceManager = new KAction(KIcon("input-mouse"), i18n("Manage devices"), menu);
    connect(deviceManager, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(deviceManager()));
    menu->addAction(deviceManager);

    KAction *configAdapter = new KAction(KIcon("audio-card"), i18n("Configure adapters"), menu);
    connect(configAdapter, SIGNAL(triggered(bool)), this, SLOT(configAdapter()));
    menu->addAction(configAdapter);

    generateDeviceEntries();

    menu->addSeparator();

    KAction *addDevice = new KAction(KIcon("edit-find-project"), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    setContextMenu(menu);
    setStandardActionsEnabled(true);
}

void Monolithic::sendFile()
{
    KProcess process;
    process.setProgram("bluedevil-sendfile");
    process.startDetached();
}

void Monolithic::addDevice()
{
    KProcess process;
    process.setProgram("bluedevil-wizard");
    process.startDetached();
}

void Monolithic::configReceive()
{
    KProcess process;
    process.startDetached("kcmshell4", QStringList("bluedeviltransfer"));
}

void Monolithic::deviceManager()
{
    KProcess process;
    process.startDetached("kcmshell4", QStringList("bluedevildevices"));
}

void Monolithic::configAdapter()
{
    KProcess process;
    process.startDetached("kcmshell4", QStringList("bluedeviladapters"));
}

void Monolithic::browseTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    Device *device = action->data().value<Device*>();
    KToolInvocation::kdeinitExec("dolphin", QStringList() << QString("obexftp:/%1/").arg(device->address().replace(':', '-')));
}

void Monolithic::sendTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    Device *device = action->data().value<Device*>();
    KToolInvocation::kdeinitExec("bluedevil-sendfile", QStringList() << QString("bluetooth://%1/").arg(device->address().replace(':', '-').toLower()));
}

void Monolithic::connectTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    Device *device = action->data().value<Device*>();
    KToolInvocation::kdeinitExec("bluedevil-audio", QStringList() << QString("bluetooth://%1/").arg(device->address().replace(':', '-').toLower()));

    action->setText(i18n("Disconnect"));
}

void Monolithic::disconnectTriggered()
{
    KAction *action = static_cast<KAction*>(sender());
    Device *device = action->data().value<Device*>();
}

void Monolithic::offlineMode()
{
    setStatus(KStatusNotifierItem::Passive);
    contextMenu()->clear();
}

Q_DECLARE_METATYPE(Device*)
