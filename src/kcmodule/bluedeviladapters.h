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

#ifndef _BLUEDEVILADAPTERS_H
#define _BLUEDEVILADAPTERS_H

#include "kded.h"

#include <QtGui/QItemSelection>

#include <kcmodule.h>

class BluetoothAdaptersModel;

class QListView;
class QCheckBox;

class KPushButton;

namespace BlueDevil {
    class Adapter;
    class Device;
}

typedef BlueDevil::Adapter Adapter;
typedef BlueDevil::Device Device;

class KCMBlueDevilAdapters
    : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilAdapters(QWidget *parent, const QVariantList&);
    virtual ~KCMBlueDevilAdapters();

    virtual void defaults();
    virtual void save();

private Q_SLOTS:
    void stateChanged(int state);
    void deviceSelectionChanged(const QItemSelection &selection);
    void trustDevice();
    void renameAliasDevice();
    void removeDevice();
    void launchWizard();

    void defaultAdapterChanged(Adapter *adapter);
    void adapterDiscoverableChanged();
    void adapterDevicesChanged(const QList<Device*> &devices);

    void updateInformationState();

private:
    void generateNoDevicesMessage();
    void fillRemoteDevicesModelInformation();

private:
    QCheckBox              *m_enable;
    KPushButton            *m_trustDevice;
    KPushButton            *m_renameAliasDevice;
    KPushButton            *m_addDevice;
    KPushButton            *m_removeDevice;
    bool                    m_isEnabled;
    BluetoothAdaptersModel *m_devicesModel;
    QListView              *m_devices;
    QWidget                *m_noDevicesMessage;
};

#endif
