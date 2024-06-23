/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <QStringList>

#include <BluezQt/PendingCall>

#include <qqmlregistration.h>

class Notify : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Notify(QObject *parent = nullptr);

public Q_SLOTS:
    // Note: We can't refer to a DeclarativeDevice in C++, and there's no API
    // to get an underlying BluezQt::DevicePtr from QML.
    void notifyIfConnectionFailed(const BluezQt::PendingCall *call, const QString &deviceName, const QString &deviceAddress);

private:
    static QString errorText(const BluezQt::PendingCall *call);
};
