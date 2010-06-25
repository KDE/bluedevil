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

#include <bluedevil/bluedevil.h>
#include <QDebug>
#include <QListWidgetItem>
#include <QListView>

using namespace BlueDevil;

DiscoverPage::DiscoverPage(QWidget* parent): QWizardPage(parent)
{
    setTitle("Discover Devices");
    setupUi(this);

    m_counter = 0;

    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(scanBtn, SIGNAL(clicked()), this, SLOT(startScan()));

    connect(Manager::self()->defaultAdapter(), SIGNAL(deviceFound(Device*)), this,
            SLOT(deviceFound(Device*)));
    connect(deviceList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this,
            SLOT(itemSelected()));
}

DiscoverPage::~DiscoverPage()
{
    delete m_timer;
}

void DiscoverPage::initializePage()
{
    startScan();
}

void DiscoverPage::cleanupPage()
{
    stopScan();
}

bool DiscoverPage::isComplete() const
{
    if (!deviceList->currentItem()) {
        return false;
    }
    return true;
}

void DiscoverPage::startScan()
{
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

    if (m_counter == 10) {
        stopScan();
    }
}

void DiscoverPage::itemSelected()
{
    emit completeChanged();
}
