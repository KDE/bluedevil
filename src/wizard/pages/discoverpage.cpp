/*
    Copyright (C) 2010 Alex Fiestas <alex@eyeos.org>
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


#include "discoverpage.h"
#include "ui_discover.h"
#include "../bluewizard.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>

#include <KDebug>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

DiscoverPage::DiscoverPage(QWidget* parent): QWizardPage(parent), m_wizard(0)
{
    setTitle("Discover Devices");
    setupUi(this);

    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
}

DiscoverPage::~DiscoverPage()
{
}

void DiscoverPage::initializePage()
{
    kDebug() << "Initialize Page";
    if (!m_wizard) {
        kDebug() << "First time in the page";
        m_wizard = static_cast<BlueWizard* >(wizard());
        connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this,
            SLOT(deviceFound(Device*)));
    }

    connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(leavePage(int)));
    startScan();
}

void DiscoverPage::leavePage(int id)
{
    if (id == 2) {
        cleanupPage();
    }
}

void DiscoverPage::cleanupPage()
{
    stopScan();
}

bool DiscoverPage::isComplete() const
{
    if (m_wizard->deviceAddress().isEmpty()) {
        return false;
    }
    return true;
}

void DiscoverPage::startScan()
{
    deviceList->clear();
    stopScan();

    Manager::self()->defaultAdapter()->startDiscovery();
}

void DiscoverPage::stopScan()
{
    if (Manager::self()->defaultAdapter()) {
        Manager::self()->defaultAdapter()->stopDiscovery();
    }
}

void DiscoverPage::deviceFound(Device* device)
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

    item->setData(Qt::UserRole, device->address());
    deviceList->addItem(item);
}

void DiscoverPage::itemSelected(QListWidgetItem* item)
{
    m_wizard->setDeviceAddress(item->data(Qt::UserRole).toByteArray());
    emit completeChanged();
}

int DiscoverPage::nextId() const
{
    if (m_wizard) {
        if (!m_wizard->deviceAddress().isEmpty()) {
            Device *device = Manager::self()->defaultAdapter()->deviceForAddress(m_wizard->deviceAddress());
            if (device->isPaired()) {
                kDebug() << "Device is paired, jumping";
                return BlueWizard::Services;
            }
        }
    }
    return BlueWizard::Pin;
}
