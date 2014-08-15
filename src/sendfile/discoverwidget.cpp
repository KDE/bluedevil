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
#include "debug_p.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QIcon>

#include <QBluez/Manager>
#include <QBluez/Adapter>
#include <QBluez/Device>

DiscoverWidget::DiscoverWidget(QBluez::Manager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_adapter(0)
{
    setupUi(this);

    connect(deviceList, &QListWidget::currentItemChanged, this, &DiscoverWidget::itemSelected);

    // Re-start scan if usable adapter changes
    connect(m_manager, &QBluez::Manager::usableAdapterChanged, this, &DiscoverWidget::startScan);

    startScan();
}

DiscoverWidget::~DiscoverWidget()
{
    stopScan();
}

void DiscoverWidget::startScan()
{
    m_itemRelation.clear();
    deviceList->clear();
    stopScan();

    m_adapter = m_manager->usableAdapter();

    if (m_adapter) {
        m_adapter->startDiscovery();
        connect(m_adapter, &QBluez::Adapter::deviceFound, this, &DiscoverWidget::deviceFound);
        connect(m_adapter, &QBluez::Adapter::deviceRemoved, this, &DiscoverWidget::deviceRemoved);

        QList<QBluez::Device*> devices = m_adapter->devices();
        Q_FOREACH (QBluez::Device *device, devices) {
            deviceFound(device);
        }
    }
}

void DiscoverWidget::stopScan()
{
    if (m_adapter) {
        m_adapter->stopDiscovery();
    }
}

void DiscoverWidget::deviceFound(QBluez::Device *device)
{
    Q_ASSERT_X(!m_itemRelation.contains(device), "DeviceFound", "Device already in item relation!");

    const QString &name = device->friendlyName().isEmpty() ? device->address() : device->friendlyName();
    const QString &icon = device->icon().isEmpty() ? QStringLiteral("preferences-system-bluetooth") : device->icon();

    connect(device, &QBluez::Device::deviceChanged, this, &DiscoverWidget::deviceChanged);

    QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(icon), name, deviceList);
    m_itemRelation.insert(device, item);
}

void DiscoverWidget::deviceRemoved(QBluez::Device *device)
{
    if (m_itemRelation.contains(device)) {
        delete m_itemRelation.value(device);
        m_itemRelation.remove(device);
    }
}

void DiscoverWidget::deviceChanged(QBluez::Device *device)
{
    if (m_itemRelation.contains(device)) {
        const QString &name = device->friendlyName().isEmpty() ? device->address() : device->friendlyName();
        const QString &icon = device->icon().isEmpty() ? QStringLiteral("preferences-system-bluetooth") : device->icon();

        QListWidgetItem *item = m_itemRelation.value(device);
        item->setText(name);
        item->setIcon(QIcon::fromTheme(icon));

        // If the device was selected but it didn't had a name, select it again
        if (deviceList->currentItem() == item) {
            itemSelected(item);
        }
    }
}

void DiscoverWidget::itemSelected(QListWidgetItem *item)
{
    emit deviceSelected(m_itemRelation.key(item));
}
