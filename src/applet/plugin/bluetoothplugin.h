/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BLUETOOTHPLUGIN_H
#define BLUETOOTHPLUGIN_H

#include <QQmlExtensionPlugin>

class BluetoothPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // BLUETOOTHPLUGIN_H
