/**
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KCModuleData>

namespace BluezQt
{
class Manager;
}

class QAction;

class BluetoothModuleData : public KCModuleData
{
    Q_OBJECT

public:
    BluetoothModuleData(QObject *parent = nullptr);

private:
    void toggleBluetooth(bool checked);
    void updateAction();

    BluezQt::Manager *m_manager;
    QAction *m_toggleAction;
};
