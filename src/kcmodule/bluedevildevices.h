/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2010 UFO Coders <info@ufocoders.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _BLUEDEVILDEVICES_H
#define _BLUEDEVILDEVICES_H

#include <kcmodule.h>

#include <BluezQt/Manager>

namespace BluezQt
{
    class DevicesModel;
}

class SystemCheck;
class DeviceDetails;

class QListView;
class QCheckBox;
class QPushButton;
class QItemSelection;

class KCMBlueDevilDevices : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilDevices(QWidget *parent, const QVariantList&);

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void deviceSelectionChanged(const QItemSelection &selection);
    void deviceDoubleClicked(const QModelIndex &index);
    void detailsDevice();
    void removeDevice();
    void connectDevice();
    void disconnectDevice();
    void launchWizard();

private:
    void generateNoDevicesMessage();

private:
    QPushButton *m_detailsDevice;
    QPushButton *m_removeDevice;
    QPushButton *m_connectDevice;
    QPushButton *m_disconnectDevice;
    QPushButton *m_addDevice;
    BluezQt::Manager *m_manager;
    BluezQt::DevicesModel *m_devicesModel;
    QListView *m_devices;
    QWidget *m_noDevicesMessage;

    SystemCheck *m_systemCheck;
    DeviceDetails *m_deviceDetails;
};

#endif
