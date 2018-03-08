/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#ifndef BLUEDEVILDEVICES_H
#define BLUEDEVILDEVICES_H

#include <kcmodule.h>

#include <QSortFilterProxyModel>

#include <BluezQt/Manager>

namespace BluezQt
{
    class DevicesModel;
}

namespace Ui
{
    class Devices;
}

class QStackedLayout;

class SystemCheck;
class DeviceDetails;

class DevicesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit DevicesProxyModel(QObject *parent = Q_NULLPTR);

    QVariant data(const QModelIndex &index, int role) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    bool duplicateIndexAddress(const QModelIndex &idx) const;
};

class KCMBlueDevilDevices : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevilDevices(QWidget *parent, const QVariantList&);
    ~KCMBlueDevilDevices() override;

    void load() override;
    void save() override;

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);

    void addDevice();
    void removeDevice();
    void currentChanged();

    void deviceAdded();
    void deviceRemoved();
    void bluetoothOperationalChanged(bool operational);

private:
    void showDeviceDetails();
    void showConfigureMessage();
    void showNoDevicesMessage();

    bool showingDeviceDetails() const;
    BluezQt::DevicePtr currentDevice() const;

    Ui::Devices *m_ui;
    BluezQt::Manager *m_manager;
    BluezQt::DevicesModel *m_devicesModel;
    DevicesProxyModel *m_proxyModel;

    SystemCheck *m_systemCheck;
    DeviceDetails *m_deviceDetails;
    QStackedLayout *m_contentLayout;
};

#endif // BLUEDEVILDEVICES_H
