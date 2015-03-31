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

#include "discover.h"
#include "ui_discover.h"
#include "../bluewizard.h"
#include "../wizardagent.h"
#include "debug_p.h"

#include <QRegExpValidator>
#include <QSortFilterProxyModel>

#include <KIconLoader>
#include <KMessageWidget>
#include <KLocalizedString>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/DevicesModel>

class DevicesProxyModel : public QSortFilterProxyModel
{
public:
    explicit DevicesProxyModel(QObject *parent);

    void setDevicesModel(BluezQt::DevicesModel *model);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;

    BluezQt::DevicePtr device(const QModelIndex &index) const;

private:
    BluezQt::DevicesModel *m_devicesModel;
};

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_devicesModel(0)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

void DevicesProxyModel::setDevicesModel(BluezQt::DevicesModel *model)
{
    m_devicesModel = model;
    setSourceModel(model);
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        return QIcon::fromTheme(index.data(BluezQt::DevicesModel::IconRole).toString());

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Move non-paired devices to the top

    bool leftPaired = left.data(BluezQt::DevicesModel::PairedRole).toBool();
    bool rightPaired = right.data(BluezQt::DevicesModel::PairedRole).toBool();

    if (leftPaired > rightPaired) {
        return true;
    } else if (leftPaired < rightPaired) {
        return false;
    }

    const QString &leftName = left.data(BluezQt::DevicesModel::FriendlyNameRole).toString();
    const QString &rightName = right.data(BluezQt::DevicesModel::FriendlyNameRole).toString();

    return QString::localeAwareCompare(leftName, rightName) > 0;
}

bool DevicesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    bool adapterPowered = index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool();
    bool adapterPairable = index.data(BluezQt::DevicesModel::AdapterPairableRole).toBool();
    return adapterPowered && adapterPairable;
}

BluezQt::DevicePtr DevicesProxyModel::device(const QModelIndex &index) const
{
    Q_ASSERT(m_devicesModel);
    return m_devicesModel->device(mapToSource(index));
}

DiscoverPage::DiscoverPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_model(0)
    , m_warningWidget(0)
{
    setupUi(this);
    setTitle(i18n("Select a device"));

    KPixmapSequenceOverlayPainter *painter = new KPixmapSequenceOverlayPainter(this);
    painter->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    painter->setWidget(working);
    painter->start();

    m_model = new DevicesProxyModel(this);
    deviceView->setModel(m_model);

    connect(deviceView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DiscoverPage::indexSelected);
}

void DiscoverPage::startDiscovery()
{
    BluezQt::AdapterPtr adapter = m_wizard->manager()->usableAdapter();
    if (adapter && !adapter->isDiscovering()) {
        adapter->startDiscovery();
    }

    m_model->setDevicesModel(new BluezQt::DevicesModel(m_wizard->manager(), this));

    checkAdapters();
    connect(m_wizard->manager(), &BluezQt::Manager::adapterAdded, this, &DiscoverPage::checkAdapters);
    connect(m_wizard->manager(), &BluezQt::Manager::adapterChanged, this, &DiscoverPage::checkAdapters);
    connect(m_wizard->manager(), &BluezQt::Manager::usableAdapterChanged, this, &DiscoverPage::usableAdapterChanged);
}

void DiscoverPage::initializePage()
{
    qCDebug(WIZARD) << "Initialize Page";

    QList <QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::NextButton;
    list << QWizard::CancelButton;
    m_wizard->setButtonLayout(list);

    QRegExp rx(QStringLiteral("[0-9]{0,9}"));
    QRegExpValidator *validator = new QRegExpValidator(rx);
    pinText->setValidator(validator);

    connect(manualPin, &QCheckBox::toggled, pinText, &QLineEdit::setEnabled);
    connect(manualPin, &QCheckBox::toggled, this, &DiscoverPage::completeChanged);
    connect(pinText, &QLineEdit::textChanged, m_wizard, &BlueWizard::setPin);
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

int DiscoverPage::nextId() const
{
    if (!isComplete()) {
        return BlueWizard::Discover;
    }

    if (!m_wizard) {
        return BlueWizard::Discover;
    }

    if (!m_wizard->device()) {
        return BlueWizard::Discover;
    }

    BluezQt::DevicePtr device = m_wizard->device();

    qCDebug(WIZARD) << "Stopping scanning";
    device->adapter()->stopDiscovery();

    if (device->isPaired()) {
        qCDebug(WIZARD) << "Device is paired, skipping to connect";
        return BlueWizard::Connect;
    }

    QString pin;
    if (manualPin->isChecked()) {
        pin = m_wizard->pin();
        m_wizard->agent()->setPin(pin);
    } else {
        pin = m_wizard->agent()->getPin(device);
    }

    qCDebug(WIZARD) << "Class: " << device->deviceType();
    qCDebug(WIZARD) << "Legacy: " << device->hasLegacyPairing();
    qCDebug(WIZARD) << "From DB: " << m_wizard->agent()->isFromDatabase();
    qCDebug(WIZARD) << "PIN: " << m_wizard->agent()->pin();

    // If keyboard no matter what, we go to the keyboard page
    if (device->deviceType() == BluezQt::Device::Keyboard) {
        qCDebug(WIZARD) << "Keyboard Pairing";
        return BlueWizard::KeyboardPairing;
    }

    // No legacy pairing and no database pin means secure pairing
    if (!device->hasLegacyPairing() && !m_wizard->agent()->isFromDatabase()) {
        qCDebug(WIZARD) << "Secure Pairing";
        return BlueWizard::SSPPairing;
    }

    // NULL pin means that no pairing is required
    if (pin == QLatin1String("NULL")) {
        qCDebug(WIZARD) << "No Pairing";
        return BlueWizard::Connect;
    }

    if (m_wizard->agent()->isFromDatabase()) {
        return BlueWizard::LegacyPairingDatabase;
    } else {
        return BlueWizard::LegacyPairing;
    }
}

void DiscoverPage::indexSelected(const QModelIndex &index)
{
    BluezQt::DevicePtr device = m_model->device(index);
    m_wizard->setDevice(device);

    Q_EMIT completeChanged();
}

void DiscoverPage::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    if (adapter && !adapter->isDiscovering()) {
        adapter->startDiscovery();
    }

    checkAdapters();
}

void DiscoverPage::checkAdapters()
{
    bool error = false;

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_wizard->manager()->adapters()) {
        if (!adapter->isPowered() || !adapter->isPairable()) {
            error = true;
            break;
        }
    }

    delete m_warningWidget;
    m_warningWidget = 0;

    if (!error) {
        return;
    }

    m_warningWidget = new KMessageWidget(this);
    m_warningWidget->setMessageType(KMessageWidget::Warning);
    m_warningWidget->setCloseButtonVisible(false);
    m_warningWidget->setText(i18n("Your Bluetooth adapter is not pairable."));

    QAction *fixAdapters = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_warningWidget);
    connect(fixAdapters, &QAction::triggered, this, &DiscoverPage::fixAdaptersError);
    m_warningWidget->addAction(fixAdapters);
    verticalLayout->insertWidget(0, m_warningWidget);
}

void DiscoverPage::fixAdaptersError()
{
    Q_FOREACH (BluezQt::AdapterPtr adapter, m_wizard->manager()->adapters()) {
        adapter->setPowered(true);
        adapter->setPairable(true);
    }
}
