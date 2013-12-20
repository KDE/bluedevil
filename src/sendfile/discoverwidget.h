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

class QTimer;
class BlueWizard;

namespace BlueDevil {
    class Device;
}
using namespace BlueDevil;

class DiscoverWidget : public QWidget
, public Ui::Discover
{
Q_OBJECT

public:
    DiscoverWidget(QWidget* parent = 0);
    virtual ~DiscoverWidget();
    void stopScan();

public Q_SLOTS:
    void startScan();

private Q_SLOTS:
    void deviceFound(const QVariantMap &deviceInfo);
    void deviceFound(Device* device);
    void itemSelected(QListWidgetItem* item);

private:
    void deviceFoundGeneric(QString address, QString name, QString icon, QString alias);

private:
    QMap<QString, QListWidgetItem*> m_itemRelation;
    BlueWizard *m_wizard;

Q_SIGNALS:
    void deviceSelected(Device *device);
};

#endif // DISCOVERWIDGET_H
