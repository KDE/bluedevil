/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

Item {
    id: bluetoothApplet

    property var connectedDevices : []
    property int runningActions : 0
    property QtObject btManager : BluezQt.Manager

    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 15
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 10

    Plasmoid.compactRepresentation: CompactRepresentation { }
    Plasmoid.fullRepresentation: FullRepresentation { }

    Plasmoid.status: (btManager.bluetoothOperational) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

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
            return i18n("Bluetooth is disabled");
        }
        if (!btManager.bluetoothOperational) {
            if (btManager.adapters.length === 0) {
                return i18n("No adapters available");
            }
            return i18n("Bluetooth is offline");
        }
        if (connectedDevices.length === 0) {
            return i18n("No connected devices");
        }
        if (connectedDevices.length === 1) {
            return i18n("%1 connected", connectedDevices[0].name);
        }
        let text = i18ncp("Number of connected devices", "%1 connected device", "%1 connected devices", connectedDevices.length);
        for (let i = 0; i < connectedDevices.length; ++i) {
            const device = connectedDevices[i];
            text += "\n \u2022 %1".arg(device.name);
        }
        return text;
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

    function toggleBluetooth()
    {
        const enable = !btManager.bluetoothOperational;
        btManager.bluetoothBlocked = !enable;

        for (let i = 0; i < btManager.adapters.length; ++i) {
            let adapter = btManager.adapters[i];
            adapter.powered = enable;
        }
    }

    function action_configure() {
        KCMShell.openSystemSettings("kcm_bluetooth");
    }

    function action_addNewDevice() {
        PlasmaBt.LaunchApp.launchWizard();
    }

    Component.onCompleted: {
        plasmoid.removeAction("configure");
        plasmoid.setAction("configure", i18n("Configure &Bluetooth…"), "configure");

        plasmoid.setAction("addNewDevice", i18n("Add New Device…"), "list-add");
        plasmoid.action("addNewDevice").visible = Qt.binding(() => {return !btManager.bluetoothBlocked;});

        updateConnectedDevices();
    }
}
