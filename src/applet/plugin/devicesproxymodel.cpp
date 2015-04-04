/*
    Copyright 2014-2015 David Rosca <nowrep@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "devicesproxymodel.h"

#include <BluezQt/Adapter>
#include <BluezQt/Device>

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

QHash<int, QByteArray> DevicesProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();
    roles[SectionRole] = QByteArrayLiteral("Section");
    roles[DeviceFullNameRole] = QByteArrayLiteral("DeviceFullName");
    return roles;
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case SectionRole:
        if (index.data(BluezQt::DevicesModel::ConnectedRole).toBool()) {
            return QStringLiteral("Connected");
        }
        return QStringLiteral("Available");

    case DeviceFullNameRole:
        if (duplicateIndexAddress(index)) {
            const QString &name = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole).toString();
            const QString &ubi = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::UbiRole).toString();
            const QString &hci = adapterHciString(ubi);

            if (!hci.isEmpty()) {
                return QStringLiteral("%1 - %2").arg(name, hci);
            }
        }
        return QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole);

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

// Returns "hciX" part from UBI "/org/bluez/hciX/dev_xx_xx_xx_xx_xx_xx"
QString DevicesProxyModel::adapterHciString(const QString &ubi) const
{
    int startIndex = ubi.indexOf(QLatin1String("/hci")) + 1;

    if (startIndex < 1) {
        return QString();
    }

    int endIndex = ubi.indexOf(QLatin1Char('/'), startIndex);

    if (endIndex == -1) {
        return ubi.mid(startIndex);
    }
    return ubi.mid(startIndex, endIndex - startIndex);
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

