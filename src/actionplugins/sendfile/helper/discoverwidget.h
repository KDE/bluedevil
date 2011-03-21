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


#ifndef DISCOVERWIDGET_H
#define DISCOVERWIDGET_H

#include "ui_discover.h"

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
    void itemSelected(QListWidgetItem* item);

private:
    QMap<QString, QListWidgetItem*> m_itemRelation;
    BlueWizard *m_wizard;

Q_SIGNALS:
    void deviceSelected(Device *device);
};

#endif // DISCOVERWIDGET_H
