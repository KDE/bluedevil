/*
 *    Copyright (C) 2010 Alejandro Fiestas Olivares  <alex@ufocoders.com>
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
#include <kmenu.h>
#include <kaction.h>
#include <kprocess.h>
#include <klocalizedstring.h>

#include <QDebug>
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

bool sortDevices(Device *device1, Device *device2)
{
    quint32 type1 = classToType(device1->deviceClass());
    quint32 type2 = classToType(device2->deviceClass());
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
        _device->setData(QVariant::fromValue<Device*>(device));
        menu->addAction(_device);
        lastDevice = device;
    }
}

void Monolithic::onlineMode()
{
    setStatus(KStatusNotifierItem::Active);

    KMenu *menu = contextMenu();

    KAction *addDevice = new KAction(KIcon("edit-find-project"), i18n("Add Device"), menu);
    connect(addDevice, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(addDevice()));
    menu->addAction(addDevice);

    KAction *configReceive = new KAction(KIcon("folder-tar"),i18n("Receive files"), menu);
    connect(configReceive, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(configReceive()));
    menu->addAction(configReceive);

    KAction *deviceManager = new KAction(KIcon("input-mouse"), i18n("Manage Devices"), menu);
    connect(deviceManager, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), this, SLOT(deviceManager()));
    menu->addAction(deviceManager);

    KAction *configAdapter = new KAction(KIcon("audio-card"), i18n("Configure Adapters"), menu);
    connect(configAdapter, SIGNAL(triggered(bool)), this, SLOT(configAdapter()));
    menu->addAction(configAdapter);

    menu->addTitle(i18n("Known Devices"));

    regenerateDeviceEntries();

    setContextMenu(menu);
    setStandardActionsEnabled(true);
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


void Monolithic::offlineMode()
{
    setStatus(KStatusNotifierItem::Passive);
    regenerateDeviceEntries();
    contextMenu()->clear();
}

Q_DECLARE_METATYPE(Device*)
