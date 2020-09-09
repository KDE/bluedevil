/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bluetoothplugin.h"
#include "devicesproxymodel.h"
#include "launchapp.h"
#include "notify.h"

#include <QQmlEngine>

static QObject *notify_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Notify;
}

static QObject *launchapp_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new LaunchApp;
}

void BluetoothPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.bluetooth"));

    qmlRegisterSingletonType<Notify>(uri, 1, 0, "Notify", notify_singleton);
    qmlRegisterSingletonType<LaunchApp>(uri, 1, 0, "LaunchApp", launchapp_singleton);
    qmlRegisterType<DevicesProxyModel>(uri, 1, 0, "DevicesProxyModel");
}
