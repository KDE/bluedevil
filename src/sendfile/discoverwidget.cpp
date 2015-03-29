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

#include <QListWidgetItem>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include <QSortFilterProxyModel>

#include <BluezQt/Device>
#include <BluezQt/Adapter>
#include <BluezQt/Services>
#include <BluezQt/DevicesModel>

class DevicesProxyModel : public QSortFilterProxyModel
{
public:
    explicit DevicesProxyModel(QObject *parent);

    void setDevicesModel(BluezQt::DevicesModel *model);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
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

bool DevicesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const QStringList &uuids = index.data(BluezQt::DevicesModel::UuidsRole).toStringList();
    bool adapterPowered = index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool();

    return adapterPowered && uuids.contains(BluezQt::Services::ObexObjectPush);
}

DiscoverWidget::DiscoverWidget(BluezQt::Manager *manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager)
{
    setupUi(this);

    m_model = new DevicesProxyModel(this);
    m_model->setDevicesModel(new BluezQt::DevicesModel(manager, this));
    devices->setModel(m_model);

    connect(devices->selectionModel(), &QItemSelectionModel::currentChanged, this, &DiscoverWidget::indexSelected);
}

BluezQt::DevicePtr DevicesProxyModel::device(const QModelIndex &index) const
{
    Q_ASSERT(m_devicesModel);
    return m_devicesModel->device(mapToSource(index));
}

void DiscoverWidget::indexSelected(const QModelIndex index)
{
    Q_EMIT deviceSelected(m_model->device(index));
}
