/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LAUNCHAPP_H
#define LAUNCHAPP_H

#include <QObject>
#include <QStringList>

class LaunchApp : public QObject
{
    Q_OBJECT

public:
    explicit LaunchApp(QObject *parent = nullptr);

public Q_SLOTS:
    bool runCommand(const QString &exe, const QStringList &args = QStringList());
};

#endif // LAUNCHAPP_H
