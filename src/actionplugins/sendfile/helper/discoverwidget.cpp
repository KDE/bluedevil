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

DiscoverWidget::DiscoverWidget(QWidget* parent)
{
    setupUi(this);

    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(QVariantMap)), this,
            SLOT(deviceFound(QVariantMap)));

    startScan();
}

DiscoverWidget::~DiscoverWidget()
{
}

void DiscoverWidget::startScan()
{
    deviceList->clear();
    stopScan();

    Manager::self()->defaultAdapter()->startDiscovery();
}

void DiscoverWidget::stopScan()
{
    if (Manager::self()->defaultAdapter()) {
        Manager::self()->defaultAdapter()->stopDiscovery();
    }
}

void DiscoverWidget::deviceFound(const QVariantMap& deviceInfo)
{
    QString address = deviceInfo["Address"].toString();
    QString name = deviceInfo["Name"].toString();
    QString icon = deviceInfo["Icon"].toString();
    QString alias = deviceInfo["Alias"].toString();

    qDebug() << "========================";
    qDebug() << "Address: " << address;
    qDebug() << "Name: " << name;
    qDebug() << "Alias: " << alias;
    qDebug() << "Icon: " << icon;
    qDebug() << "\n";


    bool origName = false;
    if (!name.isEmpty()) {
        origName = true;
    }

    if (!alias.isEmpty() && alias != name && !name.isEmpty()) {
        name = QString("%1 (%2)").arg(alias).arg(name);
    }

    if (name.isEmpty()) {
        name = address;
    }

    if (icon.isEmpty()) {
        icon.append("preferences-system-bluetooth");
    }

    if (m_itemRelation.contains(address)) {
        m_itemRelation[address]->setText(name);
        m_itemRelation[address]->setIcon(KIcon(icon));
        m_itemRelation[address]->setData(Qt::UserRole+1, origName);

        if (deviceList->currentItem() == m_itemRelation[address]) {
            emit deviceSelected(Manager::self()->defaultAdapter()->deviceForAddress(address));
        }
        return;
    }

    QListWidgetItem *item = new QListWidgetItem(KIcon(icon), name, deviceList);

    item->setData(Qt::UserRole, address);
    item->setData(Qt::UserRole+1, origName);

    m_itemRelation.insert(address, item);
}

void DiscoverWidget::itemSelected(QListWidgetItem* item)
{
    emit deviceSelected(Manager::self()->defaultAdapter()->deviceForAddress(item->data(Qt::UserRole).toString()));
}