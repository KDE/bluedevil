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

#include <bluedevil/bluedevil.h>

#include <QtCore/QAbstractItemModel>

#include <QtGui/QFontMetrics>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QCheckBox>
#include <QtGui/QListView>
#include <QtGui/QBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QStyledItemDelegate>

#include <kicon.h>
#include <kdialog.h>
#include <kprocess.h>
#include <klineedit.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

K_PLUGIN_FACTORY(BlueDevilFactory, registerPlugin<KCMBlueDevilDevices>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildevices", "bluedevil"))

////////////////////////////////////////////////////////////////////////////////////////////////////

class BluetoothDevicesModel
    : public QAbstractItemModel
{
public:
    enum ModelRoles {
        IconModelRole = 0,
        NameModelRole,
        AliasModelRole,
        DeviceTypeModelRole,
        DeviceModelRole,
        LastModelRole
    };

    BluetoothDevicesModel(QObject *parent = 0);
    virtual ~BluetoothDevicesModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    struct BluetoothDevice {
        QPixmap m_icon;
        QString m_deviceType;
        Device *m_device;
    };
    QList<BluetoothDevice> m_deviceList;
};

BluetoothDevicesModel::BluetoothDevicesModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

BluetoothDevicesModel::~BluetoothDevicesModel()
{
}

int BluetoothDevicesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant BluetoothDevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_deviceList.count()) {
        return QVariant();
    }
    switch (role) {
            case IconModelRole:
                return m_deviceList[index.row()].m_icon;
            case NameModelRole:
                return m_deviceList[index.row()].m_device->name();
            case AliasModelRole:
                return m_deviceList[index.row()].m_device->alias();
            case DeviceTypeModelRole:
                return m_deviceList[index.row()].m_deviceType;
            case DeviceModelRole:
                return QVariant::fromValue<void*>(m_deviceList[index.row()].m_device);
            default:
                break;
    }
    return QVariant();
}

bool BluetoothDevicesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_deviceList.count()) {
        return false;
    }
    switch (role) {
            case IconModelRole:
                m_deviceList[index.row()].m_icon = value.value<QPixmap>();
                break;
            case DeviceTypeModelRole:
                m_deviceList[index.row()].m_deviceType = value.toString();
                break;
            case DeviceModelRole: {
                    Device *const device = static_cast<Device*>(value.value<void*>());
                    m_deviceList[index.row()].m_device = device;
                    connect(device, SIGNAL(propertyChanged(QString,QVariant)),
                            this, SIGNAL(layoutChanged()));
                }
                break;
            default:
                return false;
    }
    emit dataChanged(index, index);
    return true;
}

QModelIndex BluetoothDevicesModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (row < 0 || row >= m_deviceList.count() || column != 0) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex BluetoothDevicesModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

int BluetoothDevicesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_deviceList.count();
}

bool BluetoothDevicesModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > m_deviceList.count() || count < 1) {
        return false;
    }
    beginInsertRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        m_deviceList.insert(i, BluetoothDevice());
    }
    endInsertRows();
    return true;
}

bool BluetoothDevicesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > m_deviceList.count() || count < 1) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        m_deviceList.removeAt(row);
    }
    endRemoveRows();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class BluetoothDevicesDelegate
    : public QStyledItemDelegate
{
public:
    BluetoothDevicesDelegate(QObject *parent = 0);
    virtual ~BluetoothDevicesDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const int smallIconSize;
    QPixmap m_blockedPixmap;
    QPixmap m_trustedPixmap;
    QPixmap m_untrustedPixmap;
    QPixmap m_connectedPixmap;
    QPixmap m_disconnectedPixmap;
};

BluetoothDevicesDelegate::BluetoothDevicesDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      smallIconSize(IconSize(KIconLoader::Toolbar))
{
    KIcon blockedIcon("dialog-cancel");
    m_blockedPixmap = blockedIcon.pixmap(smallIconSize, smallIconSize);
    KIcon trustedIcon("security-high");
    m_trustedPixmap = trustedIcon.pixmap(smallIconSize, smallIconSize);
    KIcon untrustedIcon("security-low");
    m_untrustedPixmap = untrustedIcon.pixmap(smallIconSize, smallIconSize);
    KIcon connectedIcon("user-online");
    m_connectedPixmap = connectedIcon.pixmap(smallIconSize, smallIconSize);
    KIcon disconnectedIcon("user-offline");
    m_disconnectedPixmap = disconnectedIcon.pixmap(smallIconSize, smallIconSize);
}

BluetoothDevicesDelegate::~BluetoothDevicesDelegate()
{
}

void BluetoothDevicesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    QRect r = option.rect;

// Draw icon
    const QModelIndex iconIndex = index.model()->index(index.row(), 0);
    const QPixmap icon = iconIndex.data(BluetoothDevicesModel::IconModelRole).value<QPixmap>();
    painter->drawPixmap(r.left() + 5, r.top() + (r.height() - icon.height()) / 2, icon);

// Draw alias and device type
    const QModelIndex idx = index.model()->index(index.row(), 0);
    r.setTop(r.top() + smallIconSize / 2);
    r.setBottom(r.bottom() - smallIconSize / 2);
    r.setLeft(r.left() + IconSize(KIconLoader::Dialog) * 1.8);
    QFont f = kapp->font();
    f.setBold(true);
    painter->save();
    painter->setFont(f);
    const QString name = idx.data(BluetoothDevicesModel::NameModelRole).toString();
    const QString alias = idx.data(BluetoothDevicesModel::AliasModelRole).toString();
    if (name == alias) {
        painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, name);
    } else {
        painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, QString("%1 (%2)").arg(alias).arg(name));
    }
    painter->restore();
    painter->drawText(r, Qt::AlignLeft | Qt::AlignBottom, idx.data(BluetoothDevicesModel::DeviceTypeModelRole).toString());

// Draw state
    Device *const device = static_cast<Device*>(index.data(BluetoothDevicesModel::DeviceModelRole).value<void*>());

    r = option.rect;
    r.setTop(r.top() + r.height() / 2 - smallIconSize / 2);
    r.setLeft(r.right() - 5 - smallIconSize);
    r.setSize(QSize(smallIconSize, smallIconSize));

    if (!device->isBlocked()) {
        if (device->isConnected()) {
            painter->drawPixmap(r, m_connectedPixmap);
        } else {
            painter->drawPixmap(r, m_disconnectedPixmap);
        }

        r.setLeft(r.right() - 5 - smallIconSize * 2);
        r.setSize(QSize(smallIconSize, smallIconSize));

        if (device->isTrusted()) {
            painter->drawPixmap(r, m_trustedPixmap);
        } else {
            painter->drawPixmap(r, m_untrustedPixmap);
        }
    } else {
        painter->drawPixmap(r, m_blockedPixmap);
    }

//restore
    painter->restore();
}

QSize BluetoothDevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int width = QStyledItemDelegate::sizeHint(option, index).width();
    const int height = qMax( QFontMetrics(kapp->font()).height() * 2 + QFontMetrics(kapp->font()).xHeight(), (int)(IconSize(KIconLoader::Dialog) * 1.5)) + 10;
    return QSize(width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilDevices::KCMBlueDevilDevices(QWidget *parent, const QVariantList&)
    : KCModule(BlueDevilFactory::componentData(), parent)
    , m_enable(new QCheckBox(i18n("Enable KDE Bluetooth Integration"), this))
    , m_systemCheck(new SystemCheck(this))
    , m_deviceDetails(0)
{
    KAboutData* ab = new KAboutData(
        "kcmbluedevildevices", "bluedevil", ki18n("Bluetooth Devices"), "1.0",
        ki18n("Bluetooth Devices Control Panel Module"),
        KAboutData::License_GPL, ki18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(ki18n("Rafael Fernández López"), ki18n("Developer and Maintainer"), "ereslibre@kde.org");
    setAboutData(ab);

    connect(m_systemCheck, SIGNAL(updateInformationStateRequest()),
            this, SLOT(updateInformationState()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_enable);

    m_enable->setObjectName(QString::fromUtf8("kcfg_enableGlobalBluetooth"));
    addConfig(GlobalSettings::self(), this);

    m_isEnabled = m_enable->isChecked();

    m_systemCheck->createWarnings(layout);

// Bluetooth device list
    m_devicesModel = new BluetoothDevicesModel(this);

    m_devices = new QListView(this);
    m_devices->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_devices->setItemDelegate(new BluetoothDevicesDelegate(this));
    m_devices->setModel(m_devicesModel);

    connect(m_devices->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(deviceSelectionChanged(QItemSelection)));
    connect(m_devices, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(deviceDoubleClicked(QModelIndex)));

    layout->addWidget(m_devices);

// Actions buttons
    m_detailsDevice = new KPushButton(KIcon("document-properties"), i18nc("Details of the device", "Details"));
    m_detailsDevice->setEnabled(false);
    m_removeDevice = new KPushButton(KIcon("list-remove"), i18nc("Remove a device from the list of known devices", "Remove"));
    m_removeDevice->setEnabled(false);
    m_connectDevice = new KPushButton(KIcon("network-connect"), i18n("Connect"));
    m_connectDevice->setEnabled(false);
    m_disconnectDevice = new KPushButton(KIcon("network-disconnect"), i18n("Disconnect"));
    m_disconnectDevice->setEnabled(false);
    m_addDevice = new KPushButton(KIcon("list-add"), i18n("Add Device..."));

    connect(m_detailsDevice, SIGNAL(clicked()), this, SLOT(detailsDevice()));
    connect(m_removeDevice, SIGNAL(clicked()), this, SLOT(removeDevice()));
    connect(m_disconnectDevice, SIGNAL(clicked()), this, SLOT(disconnectDevice()));
    connect(m_connectDevice, SIGNAL(clicked()), this, SLOT(connectDevice()));
    connect(m_addDevice, SIGNAL(clicked()), this, SLOT(launchWizard()));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_detailsDevice);
    hLayout->addWidget(m_removeDevice);
    hLayout->addWidget(m_connectDevice);
    hLayout->addWidget(m_disconnectDevice);
    hLayout->addStretch();
    hLayout->addWidget(m_addDevice);
    layout->addLayout(hLayout);

    setLayout(layout);

//Logic
    connect(BlueDevil::Manager::self(), SIGNAL(usableAdapterChanged(Adapter*)),
            this, SLOT(usableAdapterChanged(Adapter*)));

    BlueDevil::Adapter *const usableAdapter = BlueDevil::Manager::self()->usableAdapter();
    if (usableAdapter) {
        connect(usableAdapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
        connect(usableAdapter, SIGNAL(deviceChanged(Device*)),
                this, SLOT(adapterDevicesChanged()));
        connect(usableAdapter, SIGNAL(deviceRemoved(Device*)),
                this, SLOT(adapterDevicesChanged()));
        connect(usableAdapter, SIGNAL(deviceFound(Device*)),
                this, SLOT(adapterDevicesChanged()));
    }

    fillRemoteDevicesModelInformation();
    updateInformationState();
}

KCMBlueDevilDevices::~KCMBlueDevilDevices()
{
}

void KCMBlueDevilDevices::defaults()
{
}

void KCMBlueDevilDevices::save()
{
    KCModule::save();
    if (!m_isEnabled && m_enable->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading("bluedevil", true);
        m_systemCheck->kded()->loadModule("bluedevil");
    } else if (m_isEnabled && !m_enable->isChecked()) {
        m_systemCheck->kded()->setModuleAutoloading("bluedevil", false);
        m_systemCheck->kded()->unloadModule("bluedevil");
    }
    m_isEnabled = m_enable->isChecked();
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

    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    m_disconnectDevice->setEnabled(device->isConnected());

    if (device->isConnected()) {
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

    Device *const device = static_cast<Device*>(index.data(BluetoothDevicesModel::DeviceModelRole).value<void*>());

    m_deviceDetails = new DeviceDetails(device, this);
    m_deviceDetails->exec();
    delete m_deviceDetails;
    m_deviceDetails = 0;
}

void KCMBlueDevilDevices::detailsDevice()
{
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());

    m_deviceDetails = new DeviceDetails(device, this);
    m_deviceDetails->exec();
    delete m_deviceDetails;
    m_deviceDetails = 0;
}


void KCMBlueDevilDevices::renameAliasDevice()
{
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    KDialog *newAlias = new KDialog(this);
    QWidget *widget = new QWidget(newAlias);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(i18n("Pick a new alias for %1", device->name()), widget));
    KLineEdit *lineEdit = new KLineEdit(widget);
    lineEdit->setText(device->alias());
    lineEdit->selectAll();
    layout->addWidget(lineEdit);
    widget->setLayout(layout);
    newAlias->setMainWidget(widget);
    newAlias->setButtons(KDialog::Ok | KDialog::Cancel);
    if (newAlias->exec() == KDialog::Accepted) {
        if (lineEdit->text().isEmpty()) {
            device->setAlias(device->name());
        } else {
            device->setAlias(lineEdit->text());
        }
    }
    delete newAlias;
}

void KCMBlueDevilDevices::removeDevice()
{
    m_removeDevice->setEnabled(false);
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());

    QString ubi = device->UBI();
    if (KMessageBox::questionYesNo(this, i18n("Are you sure that you want to remove device \"%1\" from the list of known devices?", device->alias()),
                                   i18nc("Title of window that asks for confirmation when removing a device", "Device removal")) == KMessageBox::Yes) {
        QList<Device *> deviceList = BlueDevil::Manager::self()->usableAdapter()->devices();
        Q_FOREACH(Device *item, deviceList) {
            if (item->UBI() == ubi) {
                BlueDevil::Manager::self()->usableAdapter()->removeDevice(device);
                return;
            }
        }
    } else {
        m_removeDevice->setEnabled(true);
    }
}

void KCMBlueDevilDevices::connectDevice()
{
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    device->connectDevice();
}

void KCMBlueDevilDevices::disconnectDevice()
{
    m_disconnectDevice->setEnabled(false);
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    device->disconnect();
}

void KCMBlueDevilDevices::launchWizard()
{
    KProcess wizard;
    wizard.setProgram("bluedevil-wizard");
    wizard.startDetached();
}

void KCMBlueDevilDevices::usableAdapterChanged(Adapter *adapter)
{
    if (adapter) {
        connect(adapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
        connect(adapter, SIGNAL(deviceChanged(Device*)),
                this, SLOT(adapterDevicesChanged()));
        connect(adapter, SIGNAL(deviceRemoved(Device*)),
                this, SLOT(adapterDevicesChanged()));
        connect(adapter, SIGNAL(deviceFound(Device*)),
                this, SLOT(adapterDevicesChanged()));
    }
    fillRemoteDevicesModelInformation();
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
    fillRemoteDevicesModelInformation();
}

void KCMBlueDevilDevices::generateNoDevicesMessage()
{
    QGridLayout *layout = new QGridLayout;
    m_noDevicesMessage = new QWidget(this);
    m_noDevicesMessage->setMouseTracking(true);
    m_noDevicesMessage->setBackgroundRole(QPalette::Base);
    m_noDevicesMessage->setAutoFillBackground(true);
    QLabel *label = new QLabel(m_noDevicesMessage);
    label->setPixmap(KIcon("dialog-information").pixmap(128, 128));
    layout->addWidget(label, 0, 1, Qt::AlignHCenter);
    layout->addWidget(new QLabel(i18n("No remote devices have been added"), m_noDevicesMessage),
                                 1, 1, Qt::AlignHCenter);
    KPushButton *const addDevice = new KPushButton(KIcon("list-add"), i18n("Click here to add a remote device"));
    connect(addDevice, SIGNAL(clicked()), this, SLOT(launchWizard()));
    layout->addWidget(addDevice, 2, 1, Qt::AlignHCenter);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    m_noDevicesMessage->setLayout(layout);
    m_noDevicesMessage->setVisible(false);
}

void KCMBlueDevilDevices::fillRemoteDevicesModelInformation()
{
    m_devicesModel->removeRows(0, m_devicesModel->rowCount());
    Adapter *usableAdapter = BlueDevil::Manager::self()->usableAdapter();
    QList<Device*> deviceList;
    if (usableAdapter) {
        deviceList = usableAdapter->devices();
    }
    if (deviceList.isEmpty()) {
        generateNoDevicesMessage();
        m_devices->setViewport(m_noDevicesMessage);
        m_noDevicesMessage->setVisible(true);
        return;
    } else if (m_devices->viewport() == m_noDevicesMessage) {
        QWidget *viewport = new QWidget(this);
        viewport->setMouseTracking(true);
        viewport->setBackgroundRole(QPalette::Base);
        viewport->setAutoFillBackground(true);
        m_devices->setViewport(viewport);
    }
    m_devicesModel->insertRows(0, deviceList.count());
    const QSize iconSize = QSize(IconSize(KIconLoader::Dialog) * 1.5, IconSize(KIconLoader::Dialog) * 1.5);
    int i = 0;
    Q_FOREACH (Device *const device, deviceList) {
        QModelIndex index = m_devicesModel->index(i, 0);
        m_devicesModel->setData(index, KIcon(device->icon()).pixmap(iconSize), BluetoothDevicesModel::IconModelRole);
        QString deviceType;
        const quint32 type = BlueDevil::classToType(device->deviceClass());
        switch (type) {
            case BlueDevil::BLUETOOTH_TYPE_ANY:
                deviceType = i18nc("Type of device: could not be determined", "Unknown");
                break;
            case BlueDevil::BLUETOOTH_TYPE_PHONE:
                deviceType = i18nc("This device is a Phone", "Phone");
                break;
            case BlueDevil::BLUETOOTH_TYPE_MODEM:
                deviceType = i18nc("This device is a Modem", "Modem");
                break;
            case BlueDevil::BLUETOOTH_TYPE_COMPUTER:
                deviceType = i18nc("This device is a Computer", "Computer");
                break;
            case BlueDevil::BLUETOOTH_TYPE_NETWORK:
                deviceType = i18nc("This device is of type Network", "Network");
                break;
            case BlueDevil::BLUETOOTH_TYPE_HEADSET:
                deviceType = i18nc("This device is a Headset", "Headset");
                break;
            case BlueDevil::BLUETOOTH_TYPE_HEADPHONES:
                deviceType = i18nc("This device are Headphones", "Headphones");
                break;
            case BlueDevil::BLUETOOTH_TYPE_OTHER_AUDIO:
                deviceType = i18nc("This device is of type Audio", "Audio");
                break;
            case BlueDevil::BLUETOOTH_TYPE_KEYBOARD:
                deviceType = i18nc("This device is a Keyboard", "Keyboard");
                break;
            case BlueDevil::BLUETOOTH_TYPE_MOUSE:
                deviceType = i18nc("This device is a Mouse", "Mouse");
                break;
            case BlueDevil::BLUETOOTH_TYPE_CAMERA:
                deviceType = i18nc("This device is a Camera", "Camera");
                break;
            case BlueDevil::BLUETOOTH_TYPE_PRINTER:
                deviceType = i18nc("This device is a Printer", "Printer");
                break;
            case BlueDevil::BLUETOOTH_TYPE_JOYPAD:
                deviceType = i18nc("This device is a Joypad", "Joypad");
                break;
            case BlueDevil::BLUETOOTH_TYPE_TABLET:
                deviceType = i18nc("This device is a Tablet", "Tablet");
                break;
            default:
                deviceType = i18nc("Type of device: could not be determined", "Unknown");
                break;
        }
        m_devicesModel->setData(index, i18nc("Type of remote device (e.g. Camera, Mouse, Headset...)", "Type: %1", deviceType), BluetoothDevicesModel::DeviceTypeModelRole);
        m_devicesModel->setData(index, QVariant::fromValue<void*>(device), BluetoothDevicesModel::DeviceModelRole);
        ++i;
    }
}

void KCMBlueDevilDevices::updateInformationState()
{
    m_systemCheck->updateInformationState();

    m_addDevice->setEnabled(false);
    m_devices->setEnabled(false);

    if (m_isEnabled) {
        BlueDevil::Adapter *const usableAdapter = BlueDevil::Manager::self()->usableAdapter();
        if (usableAdapter) {
            m_addDevice->setEnabled(true);
            m_devices->setEnabled(true);
        }
    }
}
