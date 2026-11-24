/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 * SPDX-FileCopyrightText: 2024 Shubham Arora <shubhamarora@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

import org.kde.bluezqt as BluezQt
import org.kde.bluedevil.components as BluedevilComponents

import "script.js" as Script

KCMUtils.SimpleKCM {
    id: root

    required property BluezQt.Device device
    readonly property var batteryLabels: BluedevilComponents.BatteryUtils.batteryLabels(device)

    title: device.name

    Connections {
        target: root.device
        function onDeviceRemoved(device: BluezQt.Device): void {
            root.pop();
        }
    }

    Connections {
        target: root.device.adapter
        function onAdapterRemoved(adapter: BluezQt.Adapter): void {
            root.pop();
        }
        function onPoweredChanged(powered: bool): void {
            if (!powered) {
                root.pop();
            }
        }
    }

    function pop(): void {
        root.KCMUtils.ConfigModule.pop();
    }

    Connections {
        target: root.KCMUtils.ConfigModule
        function onNetworkAvailable(service: string, available: bool): void {
            switch (service) {
            case "dun":
                dunButton.visible = available && root.device.connected;
                break;
            case "nap":
                napButton.visible = available && root.device.connected;
                break;
            default:
                break;
            }
        }
    }

    Connections {
        target: root.device

        function onConnectedChanged(connected: bool): void {
            root.checkNetworkConnection();
        }
    }

    Component.onCompleted: {
        checkNetworkConnection();
    }

    function checkNetworkConnection(): void {
        KCMUtils.ConfigModule.checkNetworkConnection(device.uuids, device.address);
    }

    function makeCall(call: BluezQt.PendingCall): void {
        indicator.running = true
        call.finished.connect(call => {
            indicator.running = false
            if (call.error) {
                errorMessage.text = call.errorText
                errorMessage.visible = true
            }
        })
    }

    BluedevilComponents.ForgetDeviceDialog {
        id: forgetDeviceDialog

        parent: root.QQC2.Overlay.overlay

        onCall: call => {
            root.makeCall(call);
        }
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: errorMessage
        position: Kirigami.InlineMessage.Position.Header
        type: Kirigami.MessageType.Error
        showCloseButton: true
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: root.device.icon
            Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.Form {
            Layout.fillWidth: true
            Kirigami.FormGroup {
                Kirigami.FormAction {
                    action: Kirigami.Action {
                        enabled: !indicator.running && !root.device.blocked
                        text: root.device.connected
                            ? i18nc("@action:button disconnect from device", "Disconnect")
                            : i18nc("@action:button connect to device", "Connect")
                        icon.name: root.device.connected ? "network-disconnect-symbolic" : "network-connect-symbolic"


                        onTriggered: {
                            if (root.device.connected) {
                                root.makeCall(root.device.disconnectFromDevice())
                            } else {
                                root.makeCall(root.device.connectToDevice())
                            }
                        }
                    }
                    trailingItems: [
                        QQC2.BusyIndicator {
                            id: indicator
                            Layout.fillHeight: true
                            running: false
                            visible: running
                        }
                    ]
                    QQC2.ToolTip.text: i18nc("@info:tooltip", "Unblock the device first to be able to connect to it")
                    QQC2.ToolTip.visible: root.device.blocked && (hovered || activeFocus) // can't use visualFocus
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
                Kirigami.FormSeparator { }
                Kirigami.FormEntry {
                    title: i18nc("@label type of device", "Type:")
                    contentItem: QQC2.Label {
                        text: Script.deviceTypeToString(root.device)
                    }
                }
                Kirigami.FormEntry {
                    title: i18nc("@label battery charge percentage of connected device", "Battery:")
                    visible: root.batteryLabels.length > 0
                    contentItem: QQC2.Label {
                        text: root.batteryLabels.join(" · ")
                    }
                }
                Kirigami.FormEntry {
                    title: i18nc("@label bluetooth adddess of connected device", "Address:")
                    contentItem: QQC2.Label {
                        text: root.device.address
                    }
                }
                Kirigami.FormEntry {
                    title: i18nc("@label name of the adapter used to connect to device", "Adapter:")
                    contentItem: QQC2.Label {
                        text: root.device.adapter.name
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.TextField {
                        text: root.device.name
                        onTextEdited: root.device.name = text
                        Kirigami.FormData.label: i18nc("@label:textbox name of connected device, editable", "Name:")
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.Switch {
                        text: i18nc("@option:check switch, is device trusted", "Trust this device")
                        checked: root.device.trusted
                        onClicked: root.device.trusted = !root.device.trusted
                    }
                }
                Kirigami.FormEntry {
                    contentItem: QQC2.Switch {
                        text: i18nc("@option:check switch, is device blocked", "Block all incoming connections")
                        checked: root.device.blocked
                        onClicked: root.device.blocked = !root.device.blocked
                    }
                }
                Kirigami.FormSeparator {
                    visible: sendFormAction.visible || napFormAction.visible || dunFormAction.visible
                }
                Kirigami.FormAction {
                    id: sendFormAction
                    visible: root.device.uuids.includes(BluezQt.Services.ObexObjectPush) && root.device.connected
                    action: Kirigami.Action {
                        icon.name: "document-share-symbolic"
                        text: i18nc("@action:button, send file to this device", "Send File")
                        onTriggered: root.KCMUtils.ConfigModule.runSendFile(root.device.ubi)
                    }
                }
                Kirigami.FormAction {
                    id: napFormAction
                    visible: false
                    action: Kirigami.Action {
                        id: napButton
                        icon.name: "network-wireless-bluetooth-symbolic"
                        text: i18nc("@action:button", "Setup NAP Network…")
                        onTriggered: root.KCMUtils.ConfigModule.setupNetworkConnection("nap", root.device.address, root.device.name)
                    }
                }
                Kirigami.FormAction {
                    id: dunFormAction
                    visible: false
                    action: Kirigami.Action {
                        id: dunButton
                        icon.name: "network-wireless-bluetooth-symbolic"
                        text: i18nc("@action:button", "Setup DUN Network…")
                        onTriggered: root.KCMUtils.ConfigModule.setupNetworkConnection("dun", root.device.address, root.device.name)
                    }
                }
                Kirigami.FormSeparator {}
                Kirigami.FormAction {
                    action: ForgetDeviceAction {
                        dialog: forgetDeviceDialog
                        device: root.device
                        text: i18nc("@action:button Forget the Bluetooth device", "Forget");
                    }
                }
            }
        }
    }
}
