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

    property list<BluezQt.Device> connectedDevices
    property int runningActions: 0
    property alias addDeviceAction: addAction
    property alias enableBluetoothAction: enableAction

    switchWidth: Kirigami.Units.gridUnit * 15
    switchHeight: Kirigami.Units.gridUnit * 10

    // Only exists because the default CompactRepresentation doesn't expose
    // a middle-click action.
    // TODO remove once it gains that feature.
    compactRepresentation: CompactRepresentation {
        plasmoidItem: bluetoothApplet
    }

    fullRepresentation: FullRepresentation {
        hasConnectedDevices: bluetoothApplet.connectedDevices.length > 0
    }

    Plasmoid.status: (BluezQt.Manager.bluetoothOperational) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.busy: runningActions > 0

    Plasmoid.icon: {
        if (connectedDevices.length > 0) {
            return "network-bluetooth-activated-symbolic";
        }
        if (!BluezQt.Manager.bluetoothOperational) {
            return "network-bluetooth-inactive-symbolic";
        }
        return "network-bluetooth-symbolic";
    }
    toolTipMainText: i18n("Bluetooth")
    toolTipSubText: {
        if (BluezQt.Manager.bluetoothBlocked) {
            return i18n("Bluetooth is disabled; middle-click to enable");
        }
        if (!BluezQt.Manager.bluetoothOperational) {
            if (BluezQt.Manager.adapters.length === 0) {
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
            connectedDevices.forEach(device => {
                const { battery, name } = device;
                text += battery
                    ? "\n \u2022 %1 · %2".arg(name).arg(i18n("%1% Battery", battery.percentage))
                    : "\n \u2022 %1".arg(name);
            })
            text += "\n%1".arg(hint);
            return text;
        }
    }

    Connections {
        target: BluezQt.Manager

        function onDeviceAdded(): void {
            updateConnectedDevices();
        }
        function onDeviceRemoved(): void {
            updateConnectedDevices();
        }
        function onDeviceChanged(): void {
            updateConnectedDevices();
        }
        function onBluetoothBlockedChanged(): void {
            updateConnectedDevices();
        }
        function onBluetoothOperationalChanged(): void {
            updateConnectedDevices();
        }
    }

    function arraysEqualShallow(lhs: var, rhs: var): bool {
        if (lhs.length !== rhs.length) {
            return false;
        }
        return lhs.every((left, i) => left === rhs[i]);
    }

    function updateConnectedDevices(): void {
        const connectedDevices = BluezQt.Manager.devices
            .filter(device => device.connected);

        if (!arraysEqualShallow(this.connectedDevices, connectedDevices)) {
            this.connectedDevices = connectedDevices;
            connectedDevicesChanged();
        }
    }

    function toggleBluetooth(): void {
        const enable = !BluezQt.Manager.bluetoothOperational;
        BluezQt.Manager.bluetoothBlocked = !enable;

        for (let i = 0; i < BluezQt.Manager.adapters.length; ++i) {
            const adapter = BluezQt.Manager.adapters[i];
            adapter.powered = enable;
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            id: addAction
            text: i18n("Add New Device…")
            icon.name: "list-add-symbolic"
            visible: !BluezQt.Manager.bluetoothBlocked
            onTriggered: checked => PlasmaBt.LaunchApp.launchWizard()
        },
        PlasmaCore.Action {
            id: enableAction
            text: i18n("Enable Bluetooth")
            icon.name: "preferences-system-bluetooth-symbolic"
            priority: PlasmaCore.Action.LowPriority
            checkable: true
            checked: BluezQt.Manager.bluetoothOperational
            visible: BluezQt.Manager.bluetoothBlocked || BluezQt.Manager.adapters.length > 0
            onTriggered: checked => toggleBluetooth()
        }
    ]

    PlasmaCore.Action {
        id: configureAction
        text: i18n("Configure &Bluetooth…")
        icon.name: "configure-symbolic"
        onTriggered: checked => KCMUtils.KCMLauncher.openSystemSettings("kcm_bluetooth")
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction);

        updateConnectedDevices();
    }
}
