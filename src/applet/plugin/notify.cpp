/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notify.h"

#include <QIcon>

#include <KNotification>

Notify::Notify(QObject *parent)
    : QObject(parent)
{
}

void Notify::connectionFailed(const QString &title, const QString &text)
{
    KNotification *notification = new KNotification(QStringLiteral("ConnectionFailed"), KNotification::CloseOnTimeout, this);
    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(title);
    notification->setText(text);
    notification->sendEvent();
}
