/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include <bluedevil/bluedevil.h>

#include <QDebug>
#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>

using namespace BlueDevil;

DiscoverPage::DiscoverPage(QWidget* parent): QWizardPage(parent), m_counter(0), m_wizard(0)
{
    setTitle("Discover Devices");
    setupUi(this);

    m_timer = new QTimer();
    m_timer->setInterval(100);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(scanBtn, SIGNAL(clicked()), this, SLOT(startScan()));

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this,
            SLOT(deviceFound(Device*)));
    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
}

DiscoverPage::~DiscoverPage()
{
    delete m_timer;
}

void DiscoverPage::initializePage()
{
    if (!m_wizard) {
        m_wizard = static_cast<BlueWizard* >(wizard());
    }
    connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(leavePage(int)));
    startScan();
}

void DiscoverPage::leavePage(int id)
{
    if (id == 2) {
        progressBar->setValue(0);
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
    m_counter = 0;
    progressBar->setValue(0);
    deviceList->clear();
    stopScan();

    Manager::self()->defaultAdapter()->startDiscovery();
    m_timer->start();
}

void DiscoverPage::stopScan()
{
    m_counter = 0;
    m_timer->stop();
    Manager::self()->defaultAdapter()->stopDiscovery();
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

void DiscoverPage::timeout()
{
    m_counter ++;
    progressBar->setValue(m_counter);

    if (m_counter == 100) {
        stopScan();
    }
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
                return BlueWizard::Services;
            }
        }
    }
    return BlueWizard::Pin;
}
