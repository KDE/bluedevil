/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFY_H
#define NOTIFY_H

#include <QObject>
#include <QStringList>

#include <BluezQt/PendingCall>

class Notify : public QObject
{
    Q_OBJECT

public:
    explicit Notify(QObject *parent = nullptr);

public Q_SLOTS:
    void connectionFailed(const QString &title, const QString &text);
};

#endif // NOTIFY_H
