/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "devices.h"
#include "ui_devices.h"
#include "devicedetails.h"
#include "../common/systemcheck.h"
#include "version.h"

#include <QLabel>
#include <QProcess>
#include <QStackedLayout>

#include <KAboutData>
#include <KMessageBox>
#include <KIconLoader>
#include <KPluginFactory>
#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/DevicesModel>
#include <BluezQt/InitManagerJob>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedevildevices.json",
                           registerPlugin<KCMBlueDevilDevices>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (duplicateIndexAddress(index)) {
            const QString &name = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole).toString();
            const QString &ubi = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::UbiRole).toString();
            const QString &hci = DeviceDetails::adapterHciString(ubi);

            if (!hci.isEmpty()) {
                return QStringLiteral("%1 (%2)").arg(name, hci);
            }
        }
        return QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole);

    case Qt::DecorationRole:
        return QIcon::fromTheme(QSortFilterProxyModel::data(index, BluezQt::DevicesModel::IconRole).toString(), QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool leftConnected = left.data(BluezQt::DevicesModel::ConnectedRole).toBool();
    bool rightConnected = right.data(BluezQt::DevicesModel::ConnectedRole).toBool();

    if (leftConnected < rightConnected) {
        return true;
    } else if (leftConnected > rightConnected) {
        return false;
    }

    const QString &leftName = left.data(BluezQt::DevicesModel::NameRole).toString();
    const QString &rightName = right.data(BluezQt::DevicesModel::NameRole).toString();

    return QString::localeAwareCompare(leftName, rightName) > 0;
}

bool DevicesProxyModel::duplicateIndexAddress(const QModelIndex &idx) const
{
    const QModelIndexList &list = match(index(0, 0),
                                        BluezQt::DevicesModel::AddressRole,
                                        idx.data(BluezQt::DevicesModel::AddressRole).toString(),
                                        2,
                                        Qt::MatchExactly);
    return list.size() > 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilDevices::KCMBlueDevilDevices(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_ui(new Ui::Devices)
    , m_devicesModel(Q_NULLPTR)
    , m_proxyModel(Q_NULLPTR)
    , m_systemCheck(Q_NULLPTR)
{
    KAboutData *ab = new KAboutData(QStringLiteral("kcmbluedevildevices"),
                                    i18n("Bluetooth Devices"),
                                    BLUEDEVIL_VERSION,
                                    i18n("Bluetooth Devices Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));


    ab->addAuthor(QStringLiteral("David Rosca"), i18n("Maintainer"),
                  QStringLiteral("nowrep@gmail.com"), QStringLiteral("http://david.rosca.cz"));

    ab->addAuthor(QStringLiteral("Rafael Fernández López"), i18n("Previous Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));

    setAboutData(ab);
    setButtons(Apply);

    m_ui->setupUi(this);

    m_contentLayout = new QStackedLayout;
    m_deviceDetails = new DeviceDetails(this);
    m_contentLayout->addWidget(m_deviceDetails);
    m_ui->deviceDetails->setLayout(m_contentLayout);

    connect(m_ui->addButton, &QPushButton::clicked, this, &KCMBlueDevilDevices::addDevice);
    connect(m_ui->removeButton, &QPushButton::clicked, this, &KCMBlueDevilDevices::removeDevice);

    connect(m_deviceDetails, &DeviceDetails::changed, this, [this](bool state) {
        Q_EMIT changed(state);
    });

    showConfigureMessage();

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &BluezQt::InitManagerJob::result, this, &KCMBlueDevilDevices::initJobResult);
}

KCMBlueDevilDevices::~KCMBlueDevilDevices()
{
    delete m_ui;
}

void KCMBlueDevilDevices::load()
{
    if (showingDeviceDetails()) {
        m_deviceDetails->load();
    }
}

void KCMBlueDevilDevices::save()
{
    if (showingDeviceDetails()) {
        m_deviceDetails->save();
    }
}

void KCMBlueDevilDevices::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);

    const int size = IconSize(KIconLoader::Dialog);
    m_ui->deviceList->setIconSize(QSize(size, size));

    m_devicesModel = new BluezQt::DevicesModel(m_manager, this);
    m_proxyModel = new DevicesProxyModel(this);
    m_proxyModel->setSourceModel(m_devicesModel);
    m_ui->deviceList->setModel(m_proxyModel);

    if (m_manager->devices().isEmpty()) {
        showNoDevicesMessage();
        m_ui->deviceListWidget->hide();
    }

    bluetoothOperationalChanged(m_manager->isBluetoothOperational());
    connect(m_manager, &BluezQt::Manager::bluetoothOperationalChanged, this, &KCMBlueDevilDevices::bluetoothOperationalChanged);
    connect(m_manager, &BluezQt::Manager::deviceAdded, this, &KCMBlueDevilDevices::deviceAdded);
    connect(m_manager, &BluezQt::Manager::deviceRemoved, this, &KCMBlueDevilDevices::deviceRemoved);
    connect(m_ui->deviceList->selectionModel(), &QItemSelectionModel::currentChanged, this, &KCMBlueDevilDevices::currentChanged);
}

void KCMBlueDevilDevices::addDevice()
{
    QProcess::startDetached(QStringLiteral("bluedevil-wizard"));
}

void KCMBlueDevilDevices::removeDevice()
{
    BluezQt::DevicePtr device = currentDevice();
    if (!device) {
        return;
    }

    if (KMessageBox::questionYesNo(this, i18n("Are you sure that you want to remove device \"%1\" from the list of known devices?", device->friendlyName()),
                                   i18nc("Title of window that asks for confirmation when removing a device", "Device removal")) == KMessageBox::Yes) {
        device->adapter()->removeDevice(device);
    }
}

void KCMBlueDevilDevices::currentChanged()
{
    if (currentDevice()) {
        showDeviceDetails();
        m_ui->removeButton->setEnabled(true);
    } else {
        showConfigureMessage();
        m_ui->removeButton->setEnabled(false);
    }

    Q_EMIT changed(false);
}

void KCMBlueDevilDevices::deviceAdded()
{
    if (m_manager->devices().size() == 1) {
        showConfigureMessage();
    }

    m_ui->deviceListWidget->show();
}

void KCMBlueDevilDevices::deviceRemoved()
{
    if (m_manager->devices().isEmpty()) {
        showNoDevicesMessage();
        m_ui->deviceListWidget->hide();
    }
}

void KCMBlueDevilDevices::bluetoothOperationalChanged(bool operational)
{
    m_ui->addButton->setEnabled(operational);
}

void KCMBlueDevilDevices::showDeviceDetails()
{
    Q_ASSERT(currentDevice());

    m_deviceDetails->setDevice(currentDevice());
    m_contentLayout->addWidget(m_deviceDetails);
    m_contentLayout->setCurrentWidget(m_deviceDetails);
}

void KCMBlueDevilDevices::showConfigureMessage()
{
    m_contentLayout->removeWidget(m_deviceDetails);

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *label = new QLabel(i18n("Select a device to configure"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    widget->setLayout(layout);

    m_contentLayout->addWidget(widget);
    m_contentLayout->setCurrentWidget(widget);
}

void KCMBlueDevilDevices::showNoDevicesMessage()
{
    m_contentLayout->removeWidget(m_deviceDetails);

    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    QLabel *label = new QLabel(i18n("No devices found"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, 1, 1, Qt::AlignHCenter);
    QPushButton *button = new QPushButton(i18n("Add new device"));
    button->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    button->setVisible(m_manager->isBluetoothOperational());
    connect(button, &QPushButton::clicked, this, &KCMBlueDevilDevices::addDevice);
    layout->addWidget(button, 2, 1, Qt::AlignHCenter);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    widget->setLayout(layout);

    m_contentLayout->addWidget(widget);
    m_contentLayout->setCurrentWidget(widget);
}

bool KCMBlueDevilDevices::showingDeviceDetails() const
{
    return m_contentLayout->currentWidget() == m_deviceDetails;
}

BluezQt::DevicePtr KCMBlueDevilDevices::currentDevice() const
{
    const QModelIndex index = m_proxyModel->mapToSource(m_ui->deviceList->currentIndex());
    return m_devicesModel->device(index);
}

#include "devices.moc"
