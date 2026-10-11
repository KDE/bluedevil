// SPDX-FileCopyrightText: 2026 User8395 <therealuser8395@proton.me>
// SPDX-License-Identifier: SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.bluezqt as BluezQt
import org.kde.bigscreen as Bigscreen

import org.kde.plasma.private.bluetooth

Bigscreen.ScrollablePage {
    id: bluetoothView

    title: i18n("Bluetooth")
    background: null

    leftPadding: Kirigami.Units.smallSpacing
    topPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    property BluezQt.Manager manager: BluezQt.Manager

    Connections {
        target: manager

        function onUsableAdapterChanged() {
            manager.usableAdapter.startDiscovery();
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            bluetoothToggle.forceActiveFocus();
        }
    }

    DevicesProxyModel {
        id: pairedDevicesModel
        sourceModel: BluezQt.DevicesModel {}
    }

    DevicesProxyModel {
        id: unpairedDevicesModel
        unpairedOnly: true
        sourceModel: BluezQt.DevicesModel {}
    }

    Kirigami.PlaceholderMessage {
        id: noBluetoothMessage
        // We cannot use the adapter count here because that can be zero when
        // bluetooth is disabled even when there are physical devices
        visible: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown
        icon.name: "edit-none"
        text: i18n("No Bluetooth adapters found")
        explanation: i18n("Connect an external Bluetooth adapter")
        anchors.centerIn: parent
    }

    ColumnLayout {
        visible: !noBluetoothMessage.visible
        KeyNavigation.left: bluetoothView.KeyNavigation.left
        id: column
        spacing: 0

        Bigscreen.SwitchDelegate {
            id: bluetoothToggle
            text: i18n("Enable Bluetooth")
            icon.name: "network-bluetooth"
            checked: BluezQt.Manager.bluetoothOperational
            onClicked: {
                BluezQt.Manager.bluetoothBlocked = !checked;

                BluezQt.Manager.adapters.forEach(adapter => {
                    adapter.powered = checked;
                });

                checked = Qt.binding(() => BluezQt.Manager.bluetoothOperational);
            }

            KeyNavigation.down: devicesList
        }

        QQC2.Label {
            id: pairedLabel
            text: i18n("Paired devices")
            visible: BluezQt.Manager.bluetoothOperational
            font.pixelSize: Bigscreen.Units.headingFontPixelSize
            Layout.topMargin: Kirigami.Units.smallSpacing * 3
            Layout.bottomMargin: Kirigami.Units.smallSpacing * 2
        }

        ListView {
            id: pairedDelegateList
            Layout.fillWidth: true
            implicitHeight: contentHeight
            visible: BluezQt.Manager.bluetoothOperational
            KeyNavigation.down: unpairedDelegateList

            clip: true
            model: pairedDevicesModel
            delegate: DeviceDelegate {
                width: ListView.view.width

                onClicked: {
                    sidebarOverlay.delegate = this;
                    sidebarOverlay.device = model.Device;
                    sidebarOverlay.open();
                }
            }
        }

        QQC2.Label {
            id: unpairedLabel
            text: i18n("Available devices")
            visible: BluezQt.Manager.bluetoothOperational
            font.pixelSize: Bigscreen.Units.headingFontPixelSize
            Layout.topMargin: Kirigami.Units.smallSpacing * 3
            Layout.bottomMargin: Kirigami.Units.smallSpacing * 2
        }

        ListView {
            id: unpairedDelegateList
            Layout.fillWidth: true
            implicitHeight: contentHeight
            visible: BluezQt.Manager.bluetoothOperational
            KeyNavigation.up: pairedDelegateList

            clip: true
            model: unpairedDevicesModel
            delegate: DeviceDelegate {
                width: ListView.view.width

                onClicked: {
                    sidebarOverlay.delegate = this;
                    sidebarOverlay.device = model.Device;
                    sidebarOverlay.open();
                }
            }
        }

        DeviceConnectionSidebar {
            id: sidebarOverlay

            property var delegate

            onClosed: {
                if (delegate) {
                    delegate.forceActiveFocus();
                } else {
                    bluetoothToggle.forceActiveFocus();
                }
            }
        }
    }
}
