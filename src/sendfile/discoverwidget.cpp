/*****************************************************************************
 * This file is part of the KDE project                                      *
 *                                                                           *
 * Copyright (C) 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>     *
 * Copyright (C) 2010-2011 UFO Coders <info@ufocoders.com>                   *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "discoverwidget.h"
#include "ui_discover.h"
#include "debug_p.h"

#include <QAction>
#include <QSortFilterProxyModel>
#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QIcon>

#include <KMessageWidget>

#include <BluezQt/Manager>
#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/Services>
#include <BluezQt/DevicesModel>

class DevicesProxyModel : public QSortFilterProxyModel
{
public:
    explicit DevicesProxyModel(QObject *parent);

    void setDevicesModel(BluezQt::DevicesModel *model);

    QVariant data(const QModelIndex &index, int role) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

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
        return QIcon::fromTheme(index.data(BluezQt::DevicesModel::IconRole).toString(), QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const QStringList &uuids = index.data(BluezQt::DevicesModel::UuidsRole).toStringList();
    bool adapterPowered = index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool();

    return adapterPowered && uuids.contains(BluezQt::Services::ObexObjectPush);
}

BluezQt::DevicePtr DevicesProxyModel::device(const QModelIndex &index) const
{
    Q_ASSERT(m_devicesModel);
    return m_devicesModel->device(mapToSource(index));
}

DiscoverWidget::DiscoverWidget(BluezQt::Manager *manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_warningWidget(0)
{
    setupUi(this);

    m_model = new DevicesProxyModel(this);
    m_model->setDevicesModel(new BluezQt::DevicesModel(m_manager, this));
    devices->setModel(m_model);

    checkAdapters();
    connect(m_manager, &BluezQt::Manager::adapterAdded, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::adapterChanged, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::usableAdapterChanged, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, &DiscoverWidget::checkAdapters);

    connect(devices->selectionModel(), &QItemSelectionModel::currentChanged, this, &DiscoverWidget::indexSelected);
}

void DiscoverWidget::indexSelected(const QModelIndex index)
{
    Q_EMIT deviceSelected(m_model->device(index));
}

void DiscoverWidget::checkAdapters()
{
    bool error = false;

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        if (!adapter->isPowered()) {
            error = true;
            break;
        }
    }

    delete m_warningWidget;
    m_warningWidget = 0;

    if (!error && !m_manager->isBluetoothBlocked()) {
        return;
    }

    m_warningWidget = new KMessageWidget(this);
    m_warningWidget->setMessageType(KMessageWidget::Warning);
    m_warningWidget->setCloseButtonVisible(false);
    if (m_manager->isBluetoothBlocked()) {
        m_warningWidget->setText(i18n("Bluetooth is disabled."));
    } else {
        m_warningWidget->setText(i18n("Your Bluetooth adapter is powered off."));
    }

    QAction *fixAdapters = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18nc("Action to fix a problem", "Fix it"), m_warningWidget);
    connect(fixAdapters, &QAction::triggered, this, &DiscoverWidget::fixAdaptersError);
    m_warningWidget->addAction(fixAdapters);
    verticalLayout->insertWidget(0, m_warningWidget);
}

void DiscoverWidget::fixAdaptersError()
{
    m_manager->setBluetoothBlocked(false);

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        adapter->setPowered(true);
    }
}
