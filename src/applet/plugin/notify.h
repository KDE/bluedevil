/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

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
    void connectionFailed(const QString &title, const QString &text);
};
