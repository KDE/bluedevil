/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <BluezQt/PendingCall>

#include <QIdentityProxyModel>
#include <QSet>
#include <qqmlregistration.h>

#include "devicesproxymodel.h"

struct DeviceState {
    QSet<BluezQt::PendingCall *> connectingCalls;
    QSet<BluezQt::PendingCall *> disconnectingCalls;
    // TODO: Clear after timeout?
    bool connectionFailed = false;

    bool isConnecting() const;
    bool isDisconnecting() const;
};

class DevicesStateProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connecting READ isConnecting NOTIFY connectingChanged FINAL)
    Q_PROPERTY(bool disconnecting READ isDisconnecting NOTIFY disconnectingChanged FINAL)

public:
    enum Roles {
        ConnectingRole = DevicesProxyModel::LastRole + 1,
        ConnectionFailedRole,
        DisconnectingRole,
    };
    Q_ENUM(Roles)

    explicit DevicesStateProxyModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    bool isConnecting() const;
    bool isDisconnecting() const;

    // Because of wrappers and duality of C++ and declarative QML module API,
    // we can't really tell a C++ model from QML code to toggle a device,
    // because this model won't be able to issue those calls itself.
    Q_INVOKABLE void registerConnectingCallForDeviceUbi(BluezQt::PendingCall *call, const QString &ubi);
    Q_INVOKABLE void registerDisconnectingCallForDeviceUbi(BluezQt::PendingCall *call, const QString &ubi);

Q_SIGNALS:
    void connectingChanged();
    void disconnectingChanged();

private Q_SLOTS:
    void handlePendingCallFinished(BluezQt::PendingCall *call);
    void handleRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

private:
    void registerPendingCall(BluezQt::PendingCall *call, const QModelIndex &index, QSet<BluezQt::PendingCall *> &calls);
    QString ubi(const QModelIndex &index) const;
    DeviceState &state(const QModelIndex &index) const;
    QModelIndex unregisterPendingCall(BluezQt::PendingCall *call);
    QModelIndex indexByUbi(const QString &ubi);

    void notifyIfConnectionFailed(const BluezQt::PendingCall *call, const QModelIndex &index);
    static QString errorText(const BluezQt::PendingCall *call);

    // Map from Device UBI to a list of associated pending calls. Ideally,
    // should be no more than one call per device.
    mutable QMap<QString, DeviceState> m_state;
};
