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

#include <bluedevil/bluedevil.h>

#include <QtCore/QAbstractItemModel>

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
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevildevices"))

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
    QPixmap m_trustedPixmap;
    QPixmap m_untrustedPixmap;
    QPixmap m_connectedPixmap;
    QPixmap m_disconnectedPixmap;
};

BluetoothDevicesDelegate::BluetoothDevicesDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    KIcon trustedIcon("security-high");
    m_trustedPixmap = trustedIcon.pixmap(22, 22);
    KIcon untrustedIcon("security-low");
    m_untrustedPixmap = untrustedIcon.pixmap(22, 22);
    KIcon connectedIcon("user-online");
    m_connectedPixmap = connectedIcon.pixmap(22, 22);
    KIcon disconnectedIcon("user-offline");
    m_disconnectedPixmap = disconnectedIcon.pixmap(22, 22);
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

    // Draw icon
    {
        const QModelIndex iconIndex = index.model()->index(index.row(), 0);
        const QPixmap icon = iconIndex.data(BluetoothDevicesModel::IconModelRole).value<QPixmap>();
        painter->drawPixmap(option.rect.left() + 5, option.rect.top() + 5, icon);
    }

    // Draw alias and device type
    {
        const QModelIndex idx = index.model()->index(index.row(), 0);
        QRect r = option.rect;
        r.setTop(r.top() + 10);
        r.setBottom(r.bottom() - 10);
        r.setLeft(r.left() + KIconLoader::SizeLarge + 10);
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
    }

    // Draw state
    {
        Device *const device = static_cast<Device*>(index.data(BluetoothDevicesModel::DeviceModelRole).value<void*>());

        QRect r = option.rect;
        r.setTop(r.top() + r.height() / 2 - 11);
        r.setLeft(r.right() - 5 - 22);
        r.setSize(QSize(22, 22));


        if (device->isConnected()) {
            painter->drawPixmap(r, m_connectedPixmap);
        } else {
            painter->drawPixmap(r, m_disconnectedPixmap);
        }

        r.setLeft(r.right() - 5 - 22 - 22);
        r.setSize(QSize(22, 22));

        if (device->isTrusted()) {
            painter->drawPixmap(r, m_trustedPixmap);
        } else {
            painter->drawPixmap(r, m_untrustedPixmap);
        }
    }

    painter->restore();
}

QSize BluetoothDevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize res = QStyledItemDelegate::sizeHint(option, index);
    return QSize(res.width(), KIconLoader::SizeLarge + 10);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevilDevices::KCMBlueDevilDevices(QWidget *parent, const QVariantList&)
    : KCModule(BlueDevilFactory::componentData(), parent)
    , m_enable(new QCheckBox(i18n("Enable Bluetooth"), this))
    , m_systemCheck(new SystemCheck(this))
{
    KAboutData* ab = new KAboutData(
        "kcmbluedevildevices", 0, ki18n("BlueDevil Devices"), "1.0",
        ki18n("BlueDevil Devices Control Panel Module"),
        KAboutData::License_GPL, ki18n("(c) 2010 Rafael Fernández López"));

    ab->addAuthor(ki18n("Rafael Fernández López"), ki18n("Developer and Maintainer"), "ereslibre@kde.org");
    setAboutData(ab);

    connect(m_systemCheck, SIGNAL(updateInformationStateRequest()),
            this, SLOT(updateInformationState()));

    m_isEnabled = m_systemCheck->checkKDEDModuleLoaded();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_enable);
    m_systemCheck->createWarnings(layout);

    // Bluetooth device list
    {
        m_devicesModel = new BluetoothDevicesModel(this);

        m_devices = new QListView(this);
        m_devices->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_devices->setItemDelegate(new BluetoothDevicesDelegate(this));
        m_devices->setModel(m_devicesModel);

        connect(m_devices->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(deviceSelectionChanged(QItemSelection)));

        layout->addWidget(m_devices);
    }

    {
        m_trustDevice = new KPushButton(KIcon("security-high"), i18n("Trust"));
        m_trustDevice->setEnabled(false);
        m_trustDevice->setCheckable(true);
        m_renameAliasDevice = new KPushButton(KIcon("document-edit"), i18n("Rename"));
        m_renameAliasDevice->setEnabled(false);
        m_removeDevice = new KPushButton(KIcon("list-remove"), i18n("Remove"));
        m_removeDevice->setEnabled(false);
        m_addDevice = new KPushButton(KIcon("list-add"), i18n("Add Device..."));

        connect(m_trustDevice, SIGNAL(clicked()), this, SLOT(trustDevice()));
        connect(m_renameAliasDevice, SIGNAL(clicked()), this, SLOT(renameAliasDevice()));
        connect(m_removeDevice, SIGNAL(clicked()), this, SLOT(removeDevice()));
        connect(m_addDevice, SIGNAL(clicked()), this, SLOT(launchWizard()));

        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addWidget(m_trustDevice);
        hLayout->addWidget(m_renameAliasDevice);
        hLayout->addWidget(m_removeDevice);
        hLayout->addStretch();
        hLayout->addWidget(m_addDevice);
        layout->addLayout(hLayout);
    }

    setLayout(layout);

    connect(m_enable, SIGNAL(stateChanged(int)), SLOT(stateChanged(int)));
    connect(BlueDevil::Manager::self(), SIGNAL(defaultAdapterChanged(Adapter*)),
            this, SLOT(defaultAdapterChanged(Adapter*)));

    BlueDevil::Adapter *const defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (defaultAdapter) {
        connect(defaultAdapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
        connect(defaultAdapter, SIGNAL(devicesChanged(QList<Device*>)),
                this, SLOT(adapterDevicesChanged(QList<Device*>)));
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
    if (!m_isEnabled && m_enable->isChecked()) {
        m_systemCheck->kded()->loadModule("bluedevil");
    } else if (m_isEnabled && !m_enable->isChecked()) {
        m_systemCheck->kded()->unloadModule("bluedevil");
    }
    m_isEnabled = m_systemCheck->checkKDEDModuleLoaded();
    updateInformationState();
}

void KCMBlueDevilDevices::stateChanged(int)
{
    if (sender() == m_enable) {
        emit changed(m_enable->isChecked() != m_isEnabled);
    }
}

void KCMBlueDevilDevices::deviceSelectionChanged(const QItemSelection &selection)
{
    const bool enable = !selection.isEmpty();
    m_trustDevice->setEnabled(enable);
    m_trustDevice->setChecked(false);
    m_renameAliasDevice->setEnabled(enable);
    m_removeDevice->setEnabled(enable);

    if (m_devices->currentIndex().isValid()) {
        Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
        m_trustDevice->setChecked(device->isTrusted());
    }
}

void KCMBlueDevilDevices::trustDevice()
{
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    device->setTrusted(m_trustDevice->isChecked());
}

void KCMBlueDevilDevices::renameAliasDevice()
{
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    KDialog *newAlias = new KDialog(this);
    QWidget *widget = new QWidget(newAlias);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(i18n("Pick a new alias for %1").arg(device->name()), widget));
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
    Device *const device = static_cast<Device*>(m_devices->currentIndex().data(BluetoothDevicesModel::DeviceModelRole).value<void*>());
    if (KMessageBox::questionYesNo(this, i18n("Are you sure that you want to remove device \"%1\" from the list of known devices?").arg(device->alias()),
                                   i18n("Device removal")) == KMessageBox::Yes) {
        BlueDevil::Manager::self()->defaultAdapter()->removeDevice(device);
    }
}

void KCMBlueDevilDevices::launchWizard()
{
    KProcess wizard;
    wizard.setProgram("bluedevil-wizard");
    wizard.startDetached();
}

void KCMBlueDevilDevices::defaultAdapterChanged(Adapter *adapter)
{
    if (adapter) {
        connect(adapter, SIGNAL(discoverableChanged(bool)),
                this, SLOT(adapterDiscoverableChanged()));
        connect(adapter, SIGNAL(devicesChanged(QList<Device*>)),
                this, SLOT(adapterDevicesChanged(QList<Device*>)));
    }
    fillRemoteDevicesModelInformation();
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilDevices::adapterDiscoverableChanged()
{
    QTimer::singleShot(300, this, SLOT(updateInformationState()));
}

void KCMBlueDevilDevices::adapterDevicesChanged(const QList<Device*> &devices)
{
    Q_UNUSED(devices)
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
    layout->addWidget(new QLabel("No remote devices have been added", m_noDevicesMessage),
                                 1, 1, Qt::AlignHCenter);
    KPushButton *const addDevice = new KPushButton(KIcon("list-add"), "Click here to add a remote device");
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
    Adapter *defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    QList<Device*> deviceList;
    if (defaultAdapter) {
        deviceList = defaultAdapter->devices();
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
    int i = 0;
    Q_FOREACH (Device *const device, deviceList) {
        QModelIndex index = m_devicesModel->index(i, 0);
        m_devicesModel->setData(index, KIcon(device->icon()).pixmap(48, 48), BluetoothDevicesModel::IconModelRole);
        QString deviceType;
        const quint32 type = BlueDevil::classToType(device->deviceClass());
        switch (type) {
            case BlueDevil::BLUETOOTH_TYPE_ANY:
                deviceType = i18n("Unknown");
                break;
            case BlueDevil::BLUETOOTH_TYPE_PHONE:
                deviceType = i18n("Phone");
                break;
            case BlueDevil::BLUETOOTH_TYPE_MODEM:
                deviceType = i18n("Modem");
                break;
            case BlueDevil::BLUETOOTH_TYPE_COMPUTER:
                deviceType = i18n("Computer");
                break;
            case BlueDevil::BLUETOOTH_TYPE_NETWORK:
                deviceType = i18n("Network");
                break;
            case BlueDevil::BLUETOOTH_TYPE_HEADSET:
                deviceType = i18n("Headset");
                break;
            case BlueDevil::BLUETOOTH_TYPE_HEADPHONES:
                deviceType = i18n("Headphones");
                break;
            case BlueDevil::BLUETOOTH_TYPE_OTHER_AUDIO:
                deviceType = i18n("Audio");
                break;
            case BlueDevil::BLUETOOTH_TYPE_KEYBOARD:
                deviceType = i18n("Keyboard");
                break;
            case BlueDevil::BLUETOOTH_TYPE_MOUSE:
                deviceType = i18n("Mouse");
                break;
            case BlueDevil::BLUETOOTH_TYPE_CAMERA:
                deviceType = i18n("Camera");
                break;
            case BlueDevil::BLUETOOTH_TYPE_PRINTER:
                deviceType = i18n("Printer");
                break;
            case BlueDevil::BLUETOOTH_TYPE_JOYPAD:
                deviceType = i18n("Joypad");
                break;
            case BlueDevil::BLUETOOTH_TYPE_TABLET:
                deviceType = i18n("Tablet");
                break;
            default:
                deviceType = i18n("Unknown");
                break;
        }
        m_devicesModel->setData(index, i18n("Type: %1").arg(deviceType), BluetoothDevicesModel::DeviceTypeModelRole);
        m_devicesModel->setData(index, QVariant::fromValue<void*>(device), BluetoothDevicesModel::DeviceModelRole);
        ++i;
    }
}

void KCMBlueDevilDevices::updateInformationState()
{
    m_systemCheck->updateInformationState();

    m_enable->setChecked(m_isEnabled);
    m_addDevice->setEnabled(false);
    m_devices->setEnabled(false);
    if (!m_isEnabled) {
        m_addDevice->setEnabled(false);
        return;
    }
    BlueDevil::Adapter *const defaultAdapter = BlueDevil::Manager::self()->defaultAdapter();
    if (defaultAdapter && defaultAdapter->isDiscoverable() && m_systemCheck->checkNotificationsOK()) {
        m_addDevice->setEnabled(true);
        m_devices->setEnabled(true);
    }
}
