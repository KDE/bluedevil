/*
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
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

#include "bluedevil.h"

#include <bluedevil/bluedevil.h>

#include <QtCore/QAbstractItemModel>

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QCheckBox>
#include <QtGui/QListView>
#include <QtGui/QBoxLayout>
#include <QtGui/QPaintEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QStyledItemDelegate>

#include <kicon.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kcolorscheme.h>
#include <kapplication.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

K_PLUGIN_FACTORY(BlueDevilFactory, registerPlugin<KCMBlueDevil>();)
K_EXPORT_PLUGIN(BlueDevilFactory("bluedevil"))

////////////////////////////////////////////////////////////////////////////////////////////////////

class ErrorWidget
    : public QWidget
{
public:
    ErrorWidget(QWidget *parent = 0);
    virtual ~ErrorWidget();

    void setIcon(const QString &icon);
    void setReason(const QString &reason);
    void addAction(KPushButton *action);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QLabel      *m_icon;
    QLabel      *m_reason;
    QHBoxLayout *m_actions;
};

ErrorWidget::ErrorWidget(QWidget *parent)
    : QWidget(parent)
    , m_icon(new QLabel(this))
    , m_reason(new QLabel(this))
    , m_actions(new QHBoxLayout)
{
    setAutoFillBackground(false);

    m_actions->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_icon);
    layout->addWidget(m_reason, 1);

    QVBoxLayout *outter = new QVBoxLayout;
    outter->addLayout(layout);
    outter->addLayout(m_actions);

    setLayout(outter);
}

ErrorWidget::~ErrorWidget()
{
}

void ErrorWidget::setIcon(const QString &icon)
{
    m_icon->setPixmap(KIconLoader::global()->loadIcon(icon, KIconLoader::NoGroup));
}

void ErrorWidget::setReason(const QString &reason)
{
    m_reason->setText(reason);
}

void ErrorWidget::addAction(KPushButton *action)
{
    action->setAutoFillBackground(false);
    m_actions->addWidget(action);
}

void ErrorWidget::paintEvent(QPaintEvent *event)
{
    const QRect r = event->rect();
    const KColorScheme colorScheme(QPalette::Active, KColorScheme::Window);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(0, 0, r.width(), r.height(), 10, 10);
    p.fillPath(path, colorScheme.background(KColorScheme::NegativeBackground));

    QWidget::paintEvent(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class BluetoothDevicesModel
    : public QAbstractItemModel
{
public:
    enum ModelColumns {
        IconModelColumn = 0,
        AliasModelColumn,
        IdModelColumn,
        DeviceTypeModelColumn,
        LastModelColumn
    };

    BluetoothDevicesModel(QObject *parent = 0);
    virtual ~BluetoothDevicesModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    struct BluetoothDevice {
        QPixmap m_icon;
        QString m_alias;
        QString m_id;
        QString m_deviceType;
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

    return LastModelColumn;
}

QVariant BluetoothDevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_deviceList.count()) {
        return QVariant();
    }
    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case IconModelColumn:
                    return m_deviceList[index.row()].m_icon;
                case AliasModelColumn:
                    return m_deviceList[index.row()].m_alias;
                case IdModelColumn:
                    return m_deviceList[index.row()].m_id;
                case DeviceTypeModelColumn:
                    return m_deviceList[index.row()].m_deviceType;
                default:
                    break;
            }
            break;
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
        case Qt::DisplayRole:
            switch (index.column()) {
                case IconModelColumn:
                    m_deviceList[index.row()].m_icon = value.value<QPixmap>();
                    break;
                case AliasModelColumn:
                    m_deviceList[index.row()].m_alias = value.toString();
                    break;
                case IdModelColumn:
                    m_deviceList[index.row()].m_id = value.toString();
                    break;
                case DeviceTypeModelColumn:
                    m_deviceList[index.row()].m_deviceType = value.toString();
                    break;
                default:
                    return false;
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

    if (row < 0 || row >= m_deviceList.count() || column < 0 || column >= LastModelColumn) {
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
};

BluetoothDevicesDelegate::BluetoothDevicesDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
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
        const QModelIndex iconIndex = index.model()->index(index.row(), BluetoothDevicesModel::IconModelColumn);
        const QPixmap icon = iconIndex.data().value<QPixmap>();
        painter->drawPixmap(option.rect.left() + 5, option.rect.top() + 5, icon);
    }

    // Draw alias and device type
    {
        const QModelIndex aliasIndex = index.model()->index(index.row(), BluetoothDevicesModel::AliasModelColumn);
        const QModelIndex deviceTypeIndex = index.model()->index(index.row(), BluetoothDevicesModel::DeviceTypeModelColumn);
        QRect r = option.rect;
        r.setTop(r.top() + 10);
        r.setBottom(r.bottom() - 10);
        r.setLeft(r.left() + KIconLoader::SizeLarge + 10);
        QFont f = kapp->font();
        f.setBold(true);
        painter->save();
        painter->setFont(f);
        painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, aliasIndex.data().toString());
        painter->restore();
        painter->drawText(r, Qt::AlignLeft | Qt::AlignBottom, deviceTypeIndex.data().toString());
    }

    // Draw last connection
    {
        QRect r = option.rect;
        r.setTop(r.top() + 5);
        r.setRight(r.right() - 5);
        painter->drawText(r, Qt::AlignRight | Qt::AlignTop, "Last Connection: Yesterday");
    }

    painter->restore();
}

QSize BluetoothDevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize res = QStyledItemDelegate::sizeHint(option, index);
    return QSize(res.width(), KIconLoader::SizeLarge + 10);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KCMBlueDevil::KCMBlueDevil(QWidget *parent, const QVariantList&)
    : KCModule(BlueDevilFactory::componentData(), parent)
    , m_enable(new QCheckBox(i18n("Enable Bluetooth"), this))
    , m_kded(new KDED("org.kde.kded", "/kded", QDBusConnection::sessionBus()))
{
    checkKDEDModuleLoaded();

    QVBoxLayout *layout = new QVBoxLayout;

    m_noAdapters = new ErrorWidget(this);
    m_noAdapters->setIcon("window-close");
    m_noAdapters->setReason(i18n("No Bluetooth adapters have been found."));
    layout->addWidget(m_noAdapters);

    layout->addWidget(m_enable);

    // Bluetooth device list
    {
        m_devicesModel = new BluetoothDevicesModel(this);

#if 0
        // Fill some info
        {
            m_devicesModel->insertRows(0, 5);
            const QString fakeIcons[] = { "audio-headset",  "input-keyboard", "camera-web", "input-gaming", "input-tablet" };
            const QString fakeAlias[] = { "Mis casquitos", "Apple Keyboard", "Ojito Web", "Mando Wii", "Mi Tablet" };
            const QString fakeDevices[] = { "Headset", "Keyboard", "Webcam", "Wii remote control", "Tablet" };
            for (int i = 0; i < 5; ++i) {
                const QModelIndex iconIndex = m_devicesModel->index(i, BluetoothDevicesModel::IconModelColumn);
                m_devicesModel->setData(iconIndex, KIconLoader::global()->loadIcon(fakeIcons[i], KIconLoader::NoGroup));
                const QModelIndex aliasIndex = m_devicesModel->index(i, BluetoothDevicesModel::AliasModelColumn);
                m_devicesModel->setData(aliasIndex, fakeAlias[i]);
                const QModelIndex deviceTypeIndex = m_devicesModel->index(i, BluetoothDevicesModel::DeviceTypeModelColumn);
                m_devicesModel->setData(deviceTypeIndex, fakeDevices[i]);
            }
        }
#endif

        m_devices = new QListView(this);
        m_devices->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_devices->setItemDelegate(new BluetoothDevicesDelegate(this));
        m_devices->setModel(m_devicesModel);

        connect(m_devices->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(deviceSelectionChanged(QItemSelection)));

        layout->addWidget(m_devices);
    }

    {
        m_removeDevice = new KPushButton(KIcon("list-remove"), i18n("Remove Device"));
        m_removeDevice->setEnabled(false);

        connect(m_removeDevice, SIGNAL(clicked()), this, SLOT(removeDevice()));

        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addStretch();
        hLayout->addWidget(m_removeDevice);
        layout->addLayout(hLayout);
    }

    setLayout(layout);

    connect(m_enable, SIGNAL(stateChanged(int)), SLOT(stateChanged(int)));

    updateInformationState();
}

KCMBlueDevil::~KCMBlueDevil()
{
}

void KCMBlueDevil::defaults()
{
}

void KCMBlueDevil::save()
{
    if (!m_isEnabled && m_enable->isChecked()) {
        m_kded->loadModule("bluedevil");
    } else if (m_isEnabled && !m_enable->isChecked()) {
        m_kded->unloadModule("bluedevil");
    }
    checkKDEDModuleLoaded();
    updateInformationState();
}

void KCMBlueDevil::stateChanged(int)
{
    if (sender() == m_enable) {
        emit changed(m_enable->isChecked() != m_isEnabled);
    }
}

void KCMBlueDevil::deviceSelectionChanged(const QItemSelection &selection)
{
    m_removeDevice->setEnabled(!selection.isEmpty());
}

void KCMBlueDevil::removeDevice()
{
    m_devicesModel->removeRow(m_devices->currentIndex().row());
}

void KCMBlueDevil::checkKDEDModuleLoaded()
{
    const QStringList res = m_kded->loadedModules();
    bool moduleLoaded = false;
    foreach (const QString &module, res) {
        if (module == "bluedevil") {
            moduleLoaded = true;
            break;
        }
    }
    m_enable->setChecked(moduleLoaded);
    m_isEnabled = moduleLoaded;
}

void KCMBlueDevil::updateInformationState()
{
    m_noAdapters->setVisible(false);
    if (!BlueDevil::Manager::self()->defaultAdapter() && m_isEnabled) {
        m_noAdapters->setVisible(true);
        m_devices->setEnabled(false);
    }
}
