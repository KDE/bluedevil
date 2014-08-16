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
#include "../bluewizard.h"
#include "../wizardagent.h"
#include "../debug_p.h"

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QRegularExpressionValidator>

#include <KLocalizedString>
#include <kiconloader.h>
#include <kpixmapsequence.h>
#include <kpixmapsequenceoverlaypainter.h>

#include <QBluez/Manager>
#include <QBluez/Adapter>
#include <QBluez/Device>

DiscoverPage::DiscoverPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_selectedDevice(0)
    , m_wizard(parent)
{
    setupUi(this);
    setTitle(i18n("Select a device"));

    KPixmapSequenceOverlayPainter *workingPainter = new KPixmapSequenceOverlayPainter(this);
    workingPainter->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    workingPainter->setWidget(working);
    workingPainter->start();

    connect(deviceList, &QListWidget::currentItemChanged, this, &DiscoverPage::itemSelected);
}

void DiscoverPage::startDiscovery()
{
    // Re-start scan if usable adapter changes
    connect(m_wizard->manager(), &QBluez::Manager::usableAdapterChanged, this, &DiscoverPage::startScan);

    startScan();
}

void DiscoverPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Discover Page";

    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::NextButton;
    list << QWizard::CancelButton;
    m_wizard->setButtonLayout(list);

    QRegularExpression rx(QStringLiteral("[0-9]{0,9}"));
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx);
    pinText->setValidator(validator);

    connect(manualPin, &QCheckBox::toggled, pinText, &QLineEdit::setEnabled);
    connect(manualPin, &QCheckBox::toggled, this, &DiscoverPage::completeChanged);
    connect(pinText, &QLineEdit::textChanged, m_wizard, &BlueWizard::slotSetPin);
    connect(pinText, &QLineEdit::textChanged, this, &DiscoverPage::completeChanged);
}

bool DiscoverPage::isComplete() const
{
    if (!m_wizard->device()) {
        return false;
    }
    if (manualPin->isChecked() && pinText->text().isEmpty()) {
        return false;
    }
    return true;
}

void DiscoverPage::startScan()
{
    m_itemRelation.clear();
    deviceList->clear();
    stopScan();

    QBluez::Adapter *adapter = m_wizard->manager()->usableAdapter();

    if (adapter) {
        adapter->startDiscovery();
        connect(adapter, &QBluez::Adapter::deviceFound, this, &DiscoverPage::deviceFound);
        connect(adapter, &QBluez::Adapter::deviceRemoved, this, &DiscoverPage::deviceRemoved);

        QList<QBluez::Device*> devices = adapter->devices();
        Q_FOREACH (QBluez::Device *device, devices) {
            deviceFound(device);
        }
    }
}

void DiscoverPage::stopScan()
{
    if (m_wizard->manager()->usableAdapter()) {
        m_wizard->manager()->usableAdapter()->stopDiscovery();
    }
}

void DiscoverPage::deviceFound(QBluez::Device *device)
{
    const QString &name = device->friendlyName().isEmpty() ? device->address() : device->friendlyName();
    const QString &icon = device->icon().isEmpty() ? QStringLiteral("preferences-system-bluetooth") : device->icon();

    connect(device, &QBluez::Device::deviceChanged, this, &DiscoverPage::deviceChanged);

    QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(icon), name, deviceList);
    m_itemRelation.insert(device, item);

    if (!deviceList->currentItem() && device->deviceType() == QBluez::Mouse) {
        deviceList->setCurrentItem(item);
    }

    // If the device has been preselected via arguments, select it
    if (m_wizard->preselectedAddress() == device->address().toLower()) {
        deviceList->setCurrentItem(item);
    }
}

void DiscoverPage::deviceRemoved(QBluez::Device *device)
{
    if (m_itemRelation.contains(device)) {
        delete m_itemRelation.value(device);
        m_itemRelation.remove(device);
    }
}

void DiscoverPage::deviceChanged(QBluez::Device *device)
{
    if (m_itemRelation.contains(device)) {
        const QString &name = device->friendlyName().isEmpty() ? device->address() : device->friendlyName();
        const QString &icon = device->icon().isEmpty() ? QStringLiteral("preferences-system-bluetooth") : device->icon();

        QListWidgetItem *item = m_itemRelation.value(device);
        item->setText(name);
        item->setIcon(QIcon::fromTheme(icon));

        // If the device was selected but it didn't had a name, select it again
        if (deviceList->currentItem() == item) {
            itemSelected(item);
        }
    }
}

void DiscoverPage::itemSelected(QListWidgetItem* item)
{
    m_wizard->setDevice(m_itemRelation.key(item));
    emit completeChanged();
}

int DiscoverPage::nextId() const
{
    qCDebug(WIZARD) << "DiscoverPage::nextId";

    if (!isComplete()) {
        return BlueWizard::Discover;
    }

    if (!m_wizard) {
        return BlueWizard::Discover;
    }

    if (!m_wizard->device()) {
        return BlueWizard::Discover;
    }

    qCDebug(WIZARD) << "Stopping scanning";

    if (!m_wizard->manager()->usableAdapter()) {
        return BlueWizard::Fail;
    }

    m_wizard->manager()->usableAdapter()->stopDiscovery();

    QBluez::Device *device = m_wizard->device();
    if (device->isPaired()) {
        qCDebug(WIZARD) << "Device is paired, jumping to connect page";
        return BlueWizard::Connect;
    }

    QString pin;
    if (manualPin->isChecked()) {
        pin = m_wizard->pin();
        m_wizard->agent()->setPin(pin);
    } else {
        pin = m_wizard->agent()->getPin(device);
    }

    qCDebug(WIZARD) << "Type: " << QBluez::typeToString(device->deviceType());
    qCDebug(WIZARD) << "Legacy: " << device->hasLegacyPairing();
    qCDebug(WIZARD) << "From DB: " << m_wizard->agent()->isFromDatabase();
    qCDebug(WIZARD) << "PIN: " << m_wizard->agent()->pin();

    // If keyboard no matter what, we go to the keyboard page.
    if (device->deviceType() == QBluez::Keyboard) {
        qCDebug(WIZARD) << "Keyboard Pairing";
        return BlueWizard::KeyboardPairing;
    }

    if (!device->hasLegacyPairing() && !m_wizard->agent()->isFromDatabase()) {
        qCDebug(WIZARD) << "Secure Pairing";
        return BlueWizard::SSPPairing;
    }

    // If pin == NULL means that not pairing is required
    if (pin == QLatin1String("NULL")) {
        qCDebug(WIZARD) << "NO Pairing";
        return BlueWizard::Connect;
    }

    if (m_wizard->agent()->isFromDatabase()) {
        return BlueWizard::LegacyPairingDatabase;
    } else {
        return BlueWizard::LegacyPairing;
    }
}
