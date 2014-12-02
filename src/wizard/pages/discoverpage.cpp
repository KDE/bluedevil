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
#include "../wizardagent.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QRegExpValidator>

#include <KDebug>
#include <kpixmapsequenceoverlaypainter.h>

#include <bluedevil/bluedevil.h>

using namespace BlueDevil;

DiscoverPage::DiscoverPage(BlueWizard* parent): QWizardPage(parent), m_wizard(parent)
{
    setTitle(i18n("Select a device"));
    setupUi(this);

    KPixmapSequenceOverlayPainter *workingPainter = new KPixmapSequenceOverlayPainter(this);
    workingPainter->setWidget(working);
    workingPainter->start();

    connect(deviceList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this,
            SLOT(itemSelected(QListWidgetItem*)));
}

DiscoverPage::~DiscoverPage()
{
}

void DiscoverPage::initializePage()
{
    kDebug() << "Initialize Page";

    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::NextButton;
    list << QWizard::CancelButton;
    m_wizard->setButtonLayout(list);

    QRegExp rx("[0-9]{0,9}");
    QRegExpValidator *validator = new QRegExpValidator(rx);
    pinText->setValidator(validator);

    connect(Manager::self()->usableAdapter(), SIGNAL(unpairedDeviceFound(Device*)), this,
        SLOT(deviceFound(Device*)));
    connect(manualPin, SIGNAL(toggled(bool)), pinText, SLOT(setEnabled(bool)));
    connect(manualPin, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    connect(pinText, SIGNAL(textChanged(QString)), m_wizard, SLOT(setPin(QString)));
    connect(pinText, SIGNAL(textChanged(QString)), this, SIGNAL(completeChanged()));

    QMetaObject::invokeMethod(this, "startScan", Qt::QueuedConnection);
}

bool DiscoverPage::isComplete() const
{
    if (m_wizard->deviceAddress().isEmpty() || !m_wizard->device()) {
        return false;
    }
    if (manualPin->isChecked() && pinText->text().isEmpty()) {
        return false;
    }
    return true;
}

void DiscoverPage::startScan()
{
    deviceList->clear();
    stopScan();

    if (Manager::self()->usableAdapter()) {
        Manager::self()->usableAdapter()->startDiscovery();
        QList<Device*> devices = Manager::self()->usableAdapter()->devices();
        Q_FOREACH(Device *device, devices) {
            deviceFound(device);
        }
    }
}

void DiscoverPage::stopScan()
{
    if (Manager::self()->usableAdapter()) {
        Manager::self()->usableAdapter()->stopDiscovery();
    }
}

void DiscoverPage::deviceFound(Device* device)
{
    QString address = device->address();
    QString name = device->name();
    QString icon = device->icon();
    QString alias = device->alias();
    quint32 dClass = device->deviceClass();

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

        //If the device was selected but it didn't had a name, select it again
        if (deviceList->currentItem() == m_itemRelation[address]) {
            itemSelected(m_itemRelation[address]);
        }
        return;
    }

    connect(device, SIGNAL(propertyChanged(QString,QVariant)), SLOT(devicePropertyChanged()));

    QListWidgetItem *item = new QListWidgetItem(KIcon(icon), name, deviceList);

    item->setData(Qt::UserRole, address);
    item->setData(Qt::UserRole+1, origName);

    m_itemRelation.insert(address, item);

    if (!deviceList->currentItem() &&  BlueDevil::classToType(dClass) == BLUETOOTH_TYPE_MOUSE) {
        deviceList->setCurrentItem(m_itemRelation[address]);
    }

    //If the device has been preselected via arguments, select it
    if (m_wizard->preselectedAddress() == address.toLower()) {
        deviceList->setCurrentItem(m_itemRelation[address]);
    }
}

void DiscoverPage::itemSelected(QListWidgetItem* item)
{
    bool origName = item->data(Qt::UserRole+1).toBool();
    if (origName) {
        QString address = item->data(Qt::UserRole).toString();
        m_wizard->setDeviceAddress(address.toAscii());
    } else {
        m_wizard->setDeviceAddress(QByteArray());
    }
    emit completeChanged();
}

void DiscoverPage::devicePropertyChanged()
{
    Device *device = qobject_cast<Device*>(sender());
    if (device) {
        deviceFound(device);
    }
}

int DiscoverPage::nextId() const
{
    kDebug();
    if (!isComplete()) {
        return BlueWizard::Discover;
    }

    if (!m_wizard) {
        return BlueWizard::Discover;
    }

    if (m_wizard->deviceAddress().isEmpty() || !m_wizard->device()) {
        return BlueWizard::Discover;
    }

    kDebug() << "Stopping scanning";

    if (!Manager::self()->usableAdapter()) {
        return BlueWizard::Fail;
    }

    Manager::self()->usableAdapter()->stopDiscovery();
    Device *device = m_wizard->device();
    if (device->isPaired()) {
        kDebug() << "Device is paired, jumping";
        return BlueWizard::Connect;
    }

    QString pin;
    if (manualPin->isChecked()) {
        pin = m_wizard->pin();
        m_wizard->agent()->setPin(pin);
    } else {
        pin = m_wizard->agent()->getPin(device);
    }

    kDebug() << "Class: " << classToType(device->deviceClass());
    kDebug() << "Legacy: " << device->hasLegacyPairing();
    kDebug() << "From DB: " << m_wizard->agent()->isFromDatabase();
    kDebug() << "PIN: " << m_wizard->agent()->pin();

    //If keyboard no matter what, we go to the keyboard page.
    if (classToType(device->deviceClass()) == BLUETOOTH_TYPE_KEYBOARD) {
        kDebug() << "Keyboard Pairing";
        return BlueWizard::KeyboardPairing;
    }

    //If pin ==  NULL means that not pairing is required
    if (!device->hasLegacyPairing() && !m_wizard->agent()->isFromDatabase()) {
        kDebug() << "Secure Pairing";
        return BlueWizard::SSPPairing;
    }

    if (pin == "NULL") {
        kDebug() << "NO Pairing";
        return BlueWizard::NoPairing;
    }

    if (m_wizard->agent()->isFromDatabase()) {
        return BlueWizard::LegacyPairingDatabase;
    } else {
        return BlueWizard::LegacyPairing;
    }
}
