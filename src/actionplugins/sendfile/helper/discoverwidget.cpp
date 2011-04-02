/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/


#include "discoverwidget.h"
#include "ui_discover.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

DiscoverWidget::DiscoverWidget(QWidget* parent)
{
    setupUi(this);

    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this,
            SLOT(deviceFound(Device*)));

    startScan();
}

DiscoverWidget::~DiscoverWidget()
{
}

void DiscoverWidget::startScan()
{
    deviceList->clear();
    stopScan();

//     Manager::self()->defaultAdapter()->startDiscovery();
    QList <Device *> knownDevices = Manager::self()->defaultAdapter()->devices();
    Q_FOREACH(Device *device, knownDevices) {
        if (device->UUIDs().contains("00001105-0000-1000-8000-00805F9B34FB", Qt::CaseInsensitive)) {
            deviceFound(device);
        }
    }
}

void DiscoverWidget::stopScan()
{
    if (Manager::self()->defaultAdapter()) {
        Manager::self()->defaultAdapter()->stopDiscovery();
    }
}

void DiscoverWidget::deviceFound(Device* device)
{
    if (m_itemRelation.contains(device->address()) && !device->name().isEmpty()) {
        m_itemRelation[device->address()]->setText(device->friendlyName());
        if (m_itemRelation[device->address()]->isSelected()) {
            emit deviceSelected(device);
        }
        return;
    }

    QString name = device->alias();
    if (device->alias() != device->name() && !device->name().isEmpty()) {
        name.append(" ("+device->name()+")");
    }

    QString icon = device->icon();
    if (icon.isEmpty()) {
        icon.append("preferences-system-bluetooth");
    }

    QListWidgetItem *item = new QListWidgetItem(KIcon(icon), name, deviceList);

    item->setData(Qt::UserRole, qVariantFromValue<QObject*>(device));
    m_itemRelation.insert(device->address(), item);
}

void DiscoverWidget::itemSelected(QListWidgetItem* item)
{
    Device *device = qobject_cast<Device*>(item->data(Qt::UserRole).value<QObject*>());
    emit deviceSelected(device);
}