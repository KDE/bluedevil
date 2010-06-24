/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
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

#ifndef _BLUEDEVIL_H
#define _BLUEDEVIL_H

#include "kded.h"

#include <QtGui/QItemSelection>

#include <kcmodule.h>

class ErrorWidget;
class BluetoothDevicesModel;

class QListView;
class QCheckBox;

class KPushButton;

class KCMBlueDevil
    : public KCModule
{
    Q_OBJECT

public:
    KCMBlueDevil(QWidget *parent, const QVariantList&);
    virtual ~KCMBlueDevil();

    virtual void defaults();
    virtual void save();

private Q_SLOTS:
    void stateChanged(int state);
    void deviceSelectionChanged(const QItemSelection &selection);
    void removeDevice();

private:
    void checkKDEDModuleLoaded();
    void updateInformationState();

private:
    QCheckBox             *m_enable;
    KPushButton           *m_removeDevice;
    bool                   m_isEnabled;
    ErrorWidget           *m_noAdapters;
    BluetoothDevicesModel *m_devicesModel;
    QListView             *m_devices;
    KDED                  *m_kded;
};

#endif
