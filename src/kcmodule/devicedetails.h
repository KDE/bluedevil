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

#ifndef _DEVICEDETAILS_H
#define _DEVICEDETAILS_H

#include <kdialog.h>

class QCheckBox;
class KLineEdit;

namespace BlueDevil {
    class Device;
}

typedef BlueDevil::Device Device;

class DeviceDetails
    : public KDialog
{
    Q_OBJECT

public:
    DeviceDetails(Device *device, QWidget *parent = 0);
    virtual ~DeviceDetails();

protected Q_SLOTS:
    virtual void slotButtonClicked(int button);

private Q_SLOTS:
    void resetClickedSlot();
    void blockToggled(bool checked);

private:
    Device    *m_device;
    KLineEdit *m_alias;
    QCheckBox *m_blocked;
    QCheckBox *m_trusted;
};

#endif
