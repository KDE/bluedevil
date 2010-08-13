/***************************************************************************
 *   This file is part of the KDE project                                  * 
 *                                                                         *
 *   Copyright (C) 2010 Alejandro Fiestas Olivares <alex@ufocoders.com>    *
 *   Copyright (C) 2010 UFO Coders <info@ufocoders.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/


#include "discoverwidget.h"
#include "ui_discover.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

DiscoverWidget::DiscoverWidget(QWidget* parent) : m_counter(0)
{
    setupUi(this);

    m_timer = new QTimer();
    m_timer->setInterval(100);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(scanBtn, SIGNAL(clicked()), this, SLOT(startScan()));

    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this,
            SLOT(deviceFound(Device*)));

    startScan();
}

DiscoverWidget::~DiscoverWidget()
{
    delete m_timer;
}

void DiscoverWidget::timeout()
{
    m_counter ++;
    progressBar->setValue(m_counter);

    if (m_counter == 100) {
        stopScan();
    }
}

void DiscoverWidget::startScan()
{
    m_counter = 0;
    progressBar->setValue(0);
    deviceList->clear();
    stopScan();

    Manager::self()->defaultAdapter()->startDiscovery();
    m_timer->start();
}

void DiscoverWidget::stopScan()
{
    m_counter = 0;
    m_timer->stop();
    if (Manager::self()->defaultAdapter()) {
        Manager::self()->defaultAdapter()->stopDiscovery();
    }
}

void DiscoverWidget::deviceFound(Device* device)
{
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
    progressBar->setValue(m_counter);

    if (m_counter == 100) {
        stopScan();
    }
}

void DiscoverWidget::itemSelected(QListWidgetItem* item)
{
    Device *device = qobject_cast<Device*>(item->data(Qt::UserRole).value<QObject*>());
    emit deviceSelected(device);
}