/*
    Copyright (C) 2010 UFO Coders <info@ufocoders.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISCOVERPAGE_H
#define DISCOVERPAGE_H

#include "ui_discover.h"
#include <QWizardPage>

class BlueWizard;

namespace QBluez {
    class Device;
}

class DiscoverPage : public QWizardPage, public Ui::Discover
{
    Q_OBJECT

public:
    DiscoverPage(BlueWizard *parent = 0);

    void startDiscovery();

    void initializePage() Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;
    int nextId() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void startScan();
    void deviceFound(QBluez::Device *device);
    void deviceRemoved(QBluez::Device *device);
    void deviceChanged(QBluez::Device *device);
    void itemSelected(QListWidgetItem *item);

private:
    void stopScan();

private:
    QMap<QBluez::Device*, QListWidgetItem*> m_itemRelation;
    QBluez::Device *m_selectedDevice;
    BlueWizard *m_wizard;
};

#endif // DISCOVERPAGE_H
