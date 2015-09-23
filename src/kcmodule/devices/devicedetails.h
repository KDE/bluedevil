/*
 * Copyright (C) 2014-2015 David Rosca <nowrep@gmail.com>
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

#ifndef DEVICEDETAILS_H
#define DEVICEDETAILS_H

#include <QWidget>

#include <BluezQt/Types>

#include <functional>

namespace Ui {
    class DeviceDetails;
}

class DeviceDetails : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceDetails(QWidget *parent = Q_NULLPTR);

    void setDevice(BluezQt::DevicePtr device);

    void load();
    void save();

    static QString adapterHciString(const QString &ubi);

Q_SIGNALS:
    void changed(bool state);

private Q_SLOTS:
    void connectedChanged(bool connected);
    void remoteNameChanged(const QString &name);
    void uuidsChanged(const QStringList &uuids);
    void adapterNameChanged(const QString &name);

    void connectClicked();
    void sendFileClicked();
    void setupNapClicked();
    void setupDunClicked();

    void modifiedByUser();

private:
    QString deviceType() const;

    void updateActions();
    void setupNetworkConnection(const QString &service);
    void checkNetworkConnection(const QString &service, std::function<void(bool)> func);

    Ui::DeviceDetails *m_ui;
    BluezQt::DevicePtr m_device;
};

#endif // DEVICEDETAILS_H
