/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick

import org.kde.bluezqt as BluezQt
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.private.bluetooth as PlasmaBt

PlasmoidItem {
    id: bluetoothApplet

    property var connectedDevices: []
    property int runningActions: 0
    property QtObject btManager: BluezQt.Manager
    property alias addDeviceAction: addAction
    property alias enableBluetoothAction: enableAction

    switchWidth: Kirigami.Units.gridUnit * 15
    switchHeight: Kirigami.Units.gridUnit * 10

    // Only exists because the default CompactRepresentation doesn't expose
    // a middle-click action.
    // TODO remove once it gains that feature.
    compactRepresentation: CompactRepresentation { }
    fullRepresentation: FullRepresentation { }

    Plasmoid.status: (btManager.bluetoothOperational) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.busy: runningActions > 0

    Plasmoid.icon: {
        if (connectedDevices.length > 0) {
            return "network-bluetooth-activated-symbolic";
        }
        if (!btManager.bluetoothOperational) {
            return "network-bluetooth-inactive-symbolic";
        }
        return "network-bluetooth-symbolic";
    }
    toolTipMainText: i18n("Bluetooth")
    toolTipSubText: {
        if (btManager.bluetoothBlocked) {
            return i18n("Bluetooth is disabled; middle-click to enable");
        }
        if (!btManager.bluetoothOperational) {
            if (btManager.adapters.length === 0) {
                return i18n("No adapters available");
            }
            return i18n("Bluetooth is offline");
        }

        const hint = i18n("Middle-click to disable Bluetooth");

        if (connectedDevices.length === 0) {
            return "%1\n%2".arg(i18n("No connected devices")).arg(hint);

        } else if (connectedDevices.length === 1) {
            const device = connectedDevices[0];
            const battery = device.battery;
            const name = i18n("%1 connected", device.name);
            let text = battery
                ? "%1 · %2".arg(name).arg(i18n("%1% Battery", battery.percentage))
                : name
            return "%1\n%2".arg(text).arg(hint);

        } else {
            let text = i18ncp("Number of connected devices", "%1 connected device", "%1 connected devices", connectedDevices.length);
            for (let i = 0; i < connectedDevices.length; ++i) {
                const device = connectedDevices[i];
                const battery = device.battery;
                text += battery
                    ? "\n \u2022 %1 · %2".arg(device.name).arg(i18n("%1% Battery", battery.percentage))
                    : "\n \u2022 %1".arg(device.name);
            }
            text += "\n%1".arg(hint);
            return text;
        }
    }

    Connections {
        target: btManager

        function onDeviceAdded() {
            updateConnectedDevices();
        }
        function onDeviceRemoved() {
            updateConnectedDevices();
        }
        function onDeviceChanged() {
            updateConnectedDevices();
        }
        function onBluetoothBlockedChanged() {
            updateConnectedDevices();
        }
        function onBluetoothOperationalChanged() {
            updateConnectedDevices();
        }
    }

    function updateConnectedDevices() {
        let _connectedDevices = [];
        for (let i = 0; i < btManager.devices.length; ++i) {
            const device = btManager.devices[i];
            if (device.connected) {
                _connectedDevices.push(device);
            }
        }

        if (connectedDevices != _connectedDevices) {
            connectedDevices = _connectedDevices;
            connectedDevicesChanged();
        }
    }

    function toggleBluetooth() {
        const enable = !btManager.bluetoothOperational;
        btManager.bluetoothBlocked = !enable;

        for (let i = 0; i < btManager.adapters.length; ++i) {
            const adapter = btManager.adapters[i];
            adapter.powered = enable;
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            id: addAction
            text: i18n("Add New Device…")
            icon.name: "list-add-symbolic"
            visible: !btManager.bluetoothBlocked
            onTriggered: PlasmaBt.LaunchApp.launchWizard()
        },
        PlasmaCore.Action {
            id: enableAction
            text: i18n("Enable Bluetooth")
            icon.name: "preferences-system-bluetooth-symbolic"
            priority: PlasmaCore.Action.LowPriority
            checkable: true
            checked: btManager.bluetoothOperational
            visible: btManager.bluetoothBlocked || btManager.adapters.length > 0
            onTriggered: toggleBluetooth()
        }
    ]

    PlasmaCore.Action {
        id: configureAction
        text: i18n("Configure &Bluetooth…")
        icon.name: "configure-symbolic"
        onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kcm_bluetooth")
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction);

        updateConnectedDevices();
    }
}
