/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "launchapp.h"

#include <QProcess>

LaunchApp::LaunchApp(QObject *parent)
    : QObject(parent)
{
}

bool LaunchApp::runCommand(const QString &exe, const QStringList &args)
{
    return QProcess::startDetached(exe, args);
}
