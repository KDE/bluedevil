/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notify.h"

#include <QIcon>

#include <KLocalizedString>
#include <KNotification>

Notify::Notify(QObject *parent)
    : QObject(parent)
{
}

void Notify::notifyIfConnectionFailed(const BluezQt::PendingCall *call, const QString &deviceName, const QString &deviceAddress)
{
    if (call->error() == BluezQt::PendingCall::NoError) {
        return;
    }
    const auto title = i18nc("@label %1 is human-readable device name, %2 is low-level device address", "%1 (%2)", deviceName, deviceAddress);
    const auto text = errorText(call);

    KNotification *notification = new KNotification(QStringLiteral("ConnectionFailed"), KNotification::CloseOnTimeout, this);
    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(title);
    notification->setText(text);
    notification->sendEvent();
}

QString Notify::errorText(const BluezQt::PendingCall *call)
{
    switch (call->error()) {
    case BluezQt::PendingCall::Failed:
        return call->errorText() == QStringLiteral("Host is down")
            ? i18nc("Notification when the connection failed due to Failed:HostIsDown", "The device is unreachable")
            : i18nc("Notification when the connection failed due to Failed", "Connection to the device failed");

    case BluezQt::PendingCall::NotReady:
        return i18nc("Notification when the connection failed due to NotReady", "The device is not ready");

    default:
        return QString();
    }
}

#include "moc_notify.cpp"
