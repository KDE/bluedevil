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

#include "bluedevildevices.h"
#include "systemcheck.h"
#include "kded.h"
#include "globalsettings.h"
#include "devicedetails.h"
#include "debug_p.h"

#include <QLabel>
#include <QDialog>
#include <QPainter>
#include <QCheckBox>
#include <QListView>
#include <QLineEdit>
#include <QBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QFontMetrics>
#include <QApplication>
#include <QDialogButtonBox>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

#include <kprocess.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <QBluez/Manager>
#include <QBluez/InitManagerJob>
#include <QBluez/Adapter>
#include <QBluez/Device>
#include <QBluez/DevicesModel>

K_PLUGIN_FACTORY_WITH_JSON(BlueDevilFactory,
                           "bluedevildevices.json",
                           registerPlugin<KCMBlueDevilDevices>();)

////////////////////////////////////////////////////////////////////////////////////////////////////

class BluetoothDevicesDelegate : public QStyledItemDelegate
{
public:
    explicit BluetoothDevicesDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QString deviceTypeString(int type) const;

    int smallIconSize;
    QPixmap m_blockedPixmap;
    QPixmap m_trustedPixmap;
    QPixmap m_untrustedPixmap;
    QPixmap m_connectedPixmap;
    QPixmap m_disconnectedPixmap;
};

BluetoothDevicesDelegate::BluetoothDevicesDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , smallIconSize(IconSize(KIconLoader::Toolbar))
{
    m_blockedPixmap = QIcon::fromTheme(QStringLiteral("dialog-cancel")).pixmap(smallIconSize, smallIconSize);
    m_trustedPixmap = QIcon::fromTheme(QStringLiteral("security-high")).pixmap(smallIconSize, smallIconSize);
    m_untrustedPixmap = QIcon::fromTheme(QStringLiteral("security-low")).pixmap(smallIconSize, smallIconSize);
    m_connectedPixmap = QIcon::fromTheme(QStringLiteral("user-online")).pixmap(smallIconSize, smallIconSize);
    m_disconnectedPixmap = QIcon::fromTheme(QStringLiteral("user-offline")).pixmap(smallIconSize, smallIconSize);
}

void BluetoothDevicesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    QRect r = option.rect;

    // Draw icon
    const int iconSize = IconSize(KIconLoader::Dialog) * 1.5;
    const QIcon &icon = QIcon::fromTheme(index.data(QBluez::DevicesModel::IconRole).toString(),
                                         QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));
    painter->drawPixmap(r.left() + 5, r.top() + (r.height() - iconSize) / 2, icon.pixmap(iconSize));

    // Draw alias and device type
    r.setTop(r.top() + smallIconSize / 2);
    r.setBottom(r.bottom() - smallIconSize / 2);
    r.setLeft(r.left() + IconSize(KIconLoader::Dialog) * 1.8);
    QFont f = option.font;
    f.setBold(true);
    painter->save();
    painter->setFont(f);
    painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, index.data(QBluez::DevicesModel::FriendlyNameRole).toString());
    painter->restore();
    painter->drawText(r, Qt::AlignLeft | Qt::AlignBottom, deviceTypeString(index.data(QBluez::DevicesModel::DeviceTypeRole).toInt()));

    // Draw state
    r = option.rect;
    r.setTop(r.top() + r.height() / 2 - smallIconSize / 2);
    r.setLeft(r.right() - 5 - smallIconSize);
    r.setSize(QSize(smallIconSize, smallIconSize));

    if (!index.data(QBluez::DevicesModel::BlockedRole).toBool()) {
        if (index.data(QBluez::DevicesModel::ConnectedRole).toBool()) {
            painter->drawPixmap(r, m_connectedPixmap);
        } else {
            painter->drawPixmap(r, m_disconnectedPixmap);
        }

        r.setLeft(r.right() - 5 - smallIconSize * 2);
        r.setSize(QSize(smallIconSize, smallIconSize));

        if (index.data(QBluez::DevicesModel::TrustedRole).toBool()) {
            painter->drawPixmap(r, m_trustedPixmap);
        } else {
            painter->drawPixmap(r, m_untrustedPixmap);
        }
    } else {
        painter->drawPixmap(r, m_blockedPixmap);
    }

    // Restore
    painter->restore();
}

QSize BluetoothDevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int width = QStyledItemDelegate::sizeHint(option, index).width();
    const int height = qMax(option.fontMetrics.height() * 2 + option.fontMetrics.xHeight(), (int)(IconSize(KIconLoader::Dialog) * 1.5)) + 10;
    return QSize(width, height);
}

QString BluetoothDevicesDelegate::deviceTypeString(int type) const
{
    switch (type) {
        case QBluez::Device::Phone:
            return i18nc("This device is a Phone", "Phone");
        case QBluez::Device::Modem:
            return i18nc("This device is a Modem", "Modem");
        case QBluez::Device::Computer:
            return i18nc("This device is a Computer", "Computer");
        case QBluez::Device::Network:
            return i18nc("This device is of type Network", "Network");
        case QBluez::Device::Headset:
            return i18nc("This device is a Headset", "Headset");
        case QBluez::Device::Headphones:
            return i18nc("This device are Headphones", "Headphones");
        case QBluez::Device::OtherAudio:
            return i18nc("This device is of type Audio", "Audio");
        case QBluez::Device::Keyboard:
            return i18nc("This device is a Keyboard", "Keyboard");
        case QBluez::Device::Mouse:
            return i18nc("This device is a Mouse", "Mouse");
        case QBluez::Device::Camera:
            return i18nc("This device is a Camera", "Camera");
        case QBluez::Device::Printer:
            return i18nc("This device is a Printer", "Printer");
        case QBluez::Device::Joypad:
            return i18nc("This device is a Joypad", "Joypad");
        case QBluez::Device::Tablet:
            return i18nc("This device is a Tablet", "Tablet");
        default:
            return i18nc("Type of device: could not be determined", "Unknown");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilDevices::KCMBlueDevilDevices(QWidget *parent, const QVariantList&)
    : KCModule(parent)
    , m_enable(new QCheckBox(i18n("Enable KDE Bluetooth Integration"), this))
    , m_devicesModel(0)
    , m_systemCheck(0)
    , m_deviceDetails(0)
{
    KAboutData* ab = new KAboutData(QStringLiteral("kcmbluedevildevices"),
                                    i18n("Bluetooth Devices"),
                                    QStringLiteral("1.0"),
                                    i18n("Bluetooth Devices Control Panel Module"),
                                    KAboutLicense::GPL,
                                    i18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(i18n("Rafael Fernández López"), i18n("Developer and Maintainer"), QStringLiteral("ereslibre@kde.org"));
    setAboutData(ab);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_enable);

    m_enable->setObjectName(QStringLiteral("kcfg_enableGlobalBluetooth"));
    addConfig(GlobalSettings::self(), this);

    m_isEnabled = m_enable->isChecked();

    m_devices = new QListView(this);
    m_devices->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_devices->setItemDelegate(new BluetoothDevicesDelegate(this));

    connect(m_devices, &QListView::doubleClicked,
            this, &KCMBlueDevilDevices::deviceDoubleClicked);

    layout->addWidget(m_devices);

    // Actions buttons
    m_detailsDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("document-properties")), i18nc("Details of the device", "Details"));
    m_detailsDevice->setEnabled(false);
    m_removeDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18nc("Remove a device from the list of known devices", "Remove"));
    m_removeDevice->setEnabled(false);
    m_connectDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("network-connect")), i18n("Connect"));
    m_connectDevice->setEnabled(false);
    m_disconnectDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("network-disconnect")), i18n("Disconnect"));
    m_disconnectDevice->setEnabled(false);
    m_addDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add Device..."));

    connect(m_detailsDevice, &QPushButton::clicked, this, &KCMBlueDevilDevices::detailsDevice);
    connect(m_removeDevice, &QPushButton::clicked, this, &KCMBlueDevilDevices::removeDevice);
    connect(m_disconnectDevice, &QPushButton::clicked, this, &KCMBlueDevilDevices::disconnectDevice);
    connect(m_connectDevice, &QPushButton::clicked, this, &KCMBlueDevilDevices::connectDevice);
    connect(m_addDevice, &QPushButton::clicked, this, &KCMBlueDevilDevices::launchWizard);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_detailsDevice);
    hLayout->addWidget(m_removeDevice);
    hLayout->addWidget(m_connectDevice);
    hLayout->addWidget(m_disconnectDevice);
    hLayout->addStretch();
    hLayout->addWidget(m_addDevice);
    layout->addLayout(hLayout);
    setLayout(layout);

    // Initialize QBluez
    m_manager = new QBluez::Manager(this);
    QBluez::InitManagerJob *job = m_manager->init();
    job->start();
    connect(job, &QBluez::InitManagerJob::result, this, &KCMBlueDevilDevices::initJobResult);
}

void KCMBlueDevilDevices::save()
{
    KCModule::save();
    if (!m_isEnabled && m_enable->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading(QStringLiteral("bluedevil"), true);
        m_systemCheck->kded()->loadModule(QStringLiteral("bluedevil"));
    } else if (m_isEnabled && !m_enable->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading(QStringLiteral("bluedevil"), false);
        m_systemCheck->kded()->unloadModule(QStringLiteral("bluedevil"));
    }
    m_isEnabled = m_enable->isChecked();
    updateInformationState();
}

void KCMBlueDevilDevices::initJobResult(QBluez::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(KCMBLUETOOTH) << "Error initializing manager" << job->errorText();
        return;
    }

    QVBoxLayout *l = static_cast<QVBoxLayout*>(layout());

    m_systemCheck = new SystemCheck(m_manager, this);
    m_systemCheck->createWarnings(l);

    connect(m_systemCheck, &SystemCheck::updateInformationStateRequest,
            this, &KCMBlueDevilDevices::updateInformationState);

    m_devicesModel = new QBluez::DevicesModel(m_manager, this);
    m_devices->setModel(m_devicesModel);

    connect(m_devices->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &KCMBlueDevilDevices::deviceSelectionChanged);

    connect(m_manager, &QBluez::Manager::usableAdapterChanged,
            this, &KCMBlueDevilDevices::usableAdapterChanged);

    QBluez::Adapter *usableAdapter = m_manager->usableAdapter();
    if (usableAdapter) {
        connect(usableAdapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilDevices::adapterDiscoverableChanged);
        connect(usableAdapter, &QBluez::Adapter::deviceChanged,
                this, &KCMBlueDevilDevices::adapterDevicesChanged);
        connect(usableAdapter, &QBluez::Adapter::deviceRemoved,
                this, &KCMBlueDevilDevices::adapterDevicesChanged);
        connect(usableAdapter, &QBluez::Adapter::deviceFound,
                this, &KCMBlueDevilDevices::adapterDevicesChanged);
    }

    updateInformationState();
}

void KCMBlueDevilDevices::deviceSelectionChanged(const QItemSelection &selection)
{
    const bool enable = !selection.isEmpty();
    m_detailsDevice->setEnabled(enable);
    m_removeDevice->setEnabled(enable);
    m_connectDevice->setEnabled(enable);
    m_disconnectDevice->setEnabled(false);

    if (!m_devices->currentIndex().isValid()) {
        return;
    }

    bool connected = m_devices->currentIndex().data(QBluez::DevicesModel::ConnectedRole).toBool();
    m_disconnectDevice->setEnabled(connected);

    if (connected) {
        m_connectDevice->setText(i18n("Re-connect"));
    } else {
        m_connectDevice->setText(i18n("Connect"));
    }
}

void KCMBlueDevilDevices::deviceDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    m_deviceDetails = new DeviceDetails(m_devicesModel->device(index), this);
    m_deviceDetails->exec();
    delete m_deviceDetails;
    m_deviceDetails = 0;
}

void KCMBlueDevilDevices::detailsDevice()
{
    m_deviceDetails = new DeviceDetails(m_devicesModel->device(m_devices->currentIndex()), this);
    m_deviceDetails->exec();
    delete m_deviceDetails;
    m_deviceDetails = 0;
}

void KCMBlueDevilDevices::removeDevice()
{
    m_removeDevice->setEnabled(false);
    QBluez::Device *device = m_devicesModel->device(m_devices->currentIndex());

    if (KMessageBox::questionYesNo(this, i18n("Are you sure that you want to remove device \"%1\" from the list of known devices?", device->friendlyName()),
                                   i18nc("Title of window that asks for confirmation when removing a device", "Device removal")) == KMessageBox::Yes) {
        device->adapter()->removeDevice(device);
    } else {
        m_removeDevice->setEnabled(true);
    }
}

void KCMBlueDevilDevices::connectDevice()
{
    QBluez::Device *device = m_devicesModel->device(m_devices->currentIndex());
    device->connectDevice();
}

void KCMBlueDevilDevices::disconnectDevice()
{
    m_disconnectDevice->setEnabled(false);
    QBluez::Device *device = m_devicesModel->device(m_devices->currentIndex());
    device->disconnectDevice();
}

void KCMBlueDevilDevices::launchWizard()
{
    KProcess wizard;
    wizard.setProgram(QStringLiteral("bluedevil-wizard"));
    wizard.startDetached();
}

void KCMBlueDevilDevices::usableAdapterChanged(QBluez::Adapter *adapter)
{
    if (adapter) {
        connect(adapter, &QBluez::Adapter::discoverableChanged,
                this, &KCMBlueDevilDevices::adapterDiscoverableChanged);
        connect(adapter, &QBluez::Adapter::deviceChanged,
                this, &KCMBlueDevilDevices::adapterDevicesChanged);
    }
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilDevices::adapterDiscoverableChanged()
{
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilDevices::adapterDevicesChanged()
{
    if (m_deviceDetails) {
        delete m_deviceDetails;
        m_deviceDetails = 0;
    }
}

void KCMBlueDevilDevices::generateNoDevicesMessage()
{
    QGridLayout *layout = new QGridLayout;
    m_noDevicesMessage = new QWidget(this);
    m_noDevicesMessage->setMouseTracking(true);
    m_noDevicesMessage->setBackgroundRole(QPalette::Base);
    m_noDevicesMessage->setAutoFillBackground(true);
    QLabel *label = new QLabel(m_noDevicesMessage);
    label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(128, 128));
    layout->addWidget(label, 0, 1, Qt::AlignHCenter);
    layout->addWidget(new QLabel(i18n("No remote devices have been added"), m_noDevicesMessage),
                                 1, 1, Qt::AlignHCenter);
    QPushButton *const addDevice = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Click here to add a remote device"));
    connect(addDevice, SIGNAL(clicked()), this, SLOT(launchWizard()));
    layout->addWidget(addDevice, 2, 1, Qt::AlignHCenter);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    m_noDevicesMessage->setLayout(layout);
    m_noDevicesMessage->setVisible(false);
}

void KCMBlueDevilDevices::updateInformationState()
{
    m_systemCheck->updateInformationState();

    m_addDevice->setEnabled(false);
    m_devices->setEnabled(false);

    if (m_isEnabled) {
        QBluez::Adapter *usableAdapter = m_manager->usableAdapter();
        if (usableAdapter) {
            m_addDevice->setEnabled(true);
            m_devices->setEnabled(true);
        }
    }
}

#include "bluedevildevices.moc"
