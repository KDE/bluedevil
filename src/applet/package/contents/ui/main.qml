/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

import org.kde.bluezqt 1.0 as BluezQt
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: bluetoothApplet

    property var connectedDevices: []
    property int runningActions: 0
    property QtObject btManager: BluezQt.Manager

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 15
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 10

    Plasmoid.compactRepresentation: CompactRepresentation { }
    Plasmoid.fullRepresentation: FullRepresentation { }

    Plasmoid.status: (btManager.bluetoothOperational) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.busy: runningActions > 0

    Plasmoid.icon: {
        if (connectedDevices.length > 0) {
            return "preferences-system-bluetooth-activated";
        }
        if (!btManager.bluetoothOperational) {
            return "preferences-system-bluetooth-inactive";
        }
        return "preferences-system-bluetooth";
    }
    Plasmoid.toolTipMainText: i18n("Bluetooth")
    Plasmoid.toolTipSubText: {
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

    function action_configure() {
        KCMShell.openSystemSettings("kcm_bluetooth");
    }

    function action_addNewDevice() {
        PlasmaBt.LaunchApp.launchWizard();
    }

    function action_btSwitch() {
        toggleBluetooth()
    }

    Component.onCompleted: {
        Plasmoid.removeAction("configure");
        Plasmoid.setAction("configure", i18n("Configure &Bluetooth…"), "configure");

        Plasmoid.setAction("addNewDevice", i18n("Add New Device…"), "list-add");
        Plasmoid.action("addNewDevice").visible = Qt.binding(() => !btManager.bluetoothBlocked);

        Plasmoid.setAction("btSwitch", i18n("Enable Bluetooth"), "preferences-system-bluetooth");
        Plasmoid.action("btSwitch").priority = 0;
        Plasmoid.action("btSwitch").checkable = true;
        Plasmoid.action("btSwitch").checked = Qt.binding(() => btManager.bluetoothOperational);
        Plasmoid.action("btSwitch").visible = Qt.binding(() => btManager.bluetoothBlocked || btManager.adapters.length > 0);

        updateConnectedDevices();
    }
}
