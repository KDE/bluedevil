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

#ifndef DISCOVERWIDGET_H
#define DISCOVERWIDGET_H

#include "ui_discover.h"

#include <QWidget>

namespace QBluez {
    class Manager;
    class Adapter;
    class Device;
}

class DiscoverWidget : public QWidget, Ui::Discover
{
    Q_OBJECT

public:
    DiscoverWidget(QBluez::Manager *manager, QWidget *parent = 0);
    ~DiscoverWidget();

public Q_SLOTS:
    void startScan();
    void stopScan();

private Q_SLOTS:
    void deviceFound(QBluez::Device *device);
    void deviceRemoved(QBluez::Device *device);
    void deviceChanged(QBluez::Device *device);
    void itemSelected(QListWidgetItem *item);

private:
    void deviceFoundGeneric(QString address, QString name, QString icon, QString alias);

private:
    QMap<QBluez::Device*, QListWidgetItem*> m_itemRelation;
    QBluez::Manager *m_manager;
    QBluez::Adapter *m_adapter;

Q_SIGNALS:
    void deviceSelected(QBluez::Device *device);
};

#endif // DISCOVERWIDGET_H
