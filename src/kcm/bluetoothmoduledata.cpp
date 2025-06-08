/**
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "bluetoothmoduledata.h"

#include <QAction>

#include <BluezQt/Adapter>
#include <BluezQt/InitManagerJob>
#include <BluezQt/Manager>

#include <KLocalizedString>

BluetoothModuleData::BluetoothModuleData(QObject *parent)
    : KCModuleData(parent)
    , m_manager(new BluezQt::Manager(this))
    , m_toggleAction(new QAction(i18nc("@info:tooltip", "Enable Bluetooth"), this))
{
    // TODO disconnect loaded signal and wait for init job to finish? That's what kcmkeys does.
    auto *initJob = m_manager->init();
    initJob->start();

    m_toggleAction->setCheckable(true);
    connect(m_toggleAction, &QAction::triggered, this, &BluetoothModuleData::toggleBluetooth);
    setAuxiliaryAction(m_toggleAction);

    connect(m_manager, &BluezQt::Manager::bluetoothOperationalChanged, this, &BluetoothModuleData::updateAction);
    connect(m_manager->rfkill(), &BluezQt::Rfkill::stateChanged, this, &BluetoothModuleData::updateAction);

    updateAction();
}

void BluetoothModuleData::toggleBluetooth(bool checked)
{
    Q_UNUSED(checked);
    const bool enable = !m_manager->isBluetoothOperational();

    m_manager->setBluetoothBlocked(!enable);

    const auto adapters = m_manager->adapters();
    for (auto adapter : adapters) {
        adapter->setPowered(enable);
    }
}

void BluetoothModuleData::updateAction()
{
    m_toggleAction->setVisible(m_manager->rfkill()->state() != BluezQt::Rfkill::Unknown);
    m_toggleAction->setChecked(m_manager->isBluetoothOperational());
}

#include "moc_bluetoothmoduledata.cpp"
