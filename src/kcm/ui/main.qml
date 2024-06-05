/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Tom Zander <tom@flowee.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kcmutils as KCMUtils

import org.kde.bluezqt as BluezQt

import org.kde.plasma.private.bluetooth

import "script.js" as Script

KCMUtils.ScrollViewKCM {
    id: root

    actions: [
        Kirigami.Action {
            id: enableAction
            text: i18nc("@action: button as in, 'enable Bluetooth'", "Enabled")
            icon.name: "network-bluetooth-symbolic"
            checkable: true
            checked: BluezQt.Manager.bluetoothOperational
            visible: BluezQt.Manager.rfkill.state !== BluezQt.Rfkill.Unknown
            onToggled: source => {
                root.setBluetoothEnabled(!BluezQt.Manager.bluetoothOperational)
            }
            displayComponent: QQC2.Switch {
                action: enableAction
            }
        },
        Kirigami.Action {
            text: i18n("Add New Device…")
            icon.name: "list-add-symbolic"
            onTriggered: root.KCMUtils.ConfigModule.runWizard()
            visible: BluezQt.Manager.bluetoothOperational
        },
        Kirigami.Action {
            text: i18n("Configure…")
            icon.name: "configure-symbolic"
            onTriggered: root.KCMUtils.ConfigModule.push("General.qml")
            visible: BluezQt.Manager.bluetoothOperational
        }
    ]

    function makeCall(call: BluezQt.PendingCall): void {
        busyIndicator.running = true
        call.finished.connect(call => {
            busyIndicator.running = false
            if (call.error) {
                errorMessage.text = call.errorText
                errorMessage.visible = true
            }
        })
    }

    Connections {
        target: root.KCMUtils.ConfigModule

        function onErrorOccured(errorText: string): void {
            errorMessage.text = errorText
            errorMessage.visible = true
        }
    }

    Component {
        id: forgetDialogComponent

        Kirigami.PromptDialog {
            id: dialog

            required property BluezQt.Adapter adapter
            required property BluezQt.Device device
            required property string name

            signal call(BluezQt.PendingCall pc)

            title: i18n("Forget this Device?")
            subtitle: i18n("Are you sure you want to forget \"%1\"?", name)

            showCloseButton: false

            // Need to use fully custom actions because it's not possible to override
            // the text and icon of a single standardbutton, and if we use just a
            // custom action for that one, then it's in the wrong visual position
            // relative to the StandardButton-provided Cancel button
            standardButtons: Kirigami.Dialog.NoButton
            customFooterActions: [
                Kirigami.Action {
                    text: i18nc("@action:button", "Forget Device")
                    icon.name: "edit-delete-remove-symbolic"
                    onTriggered: {
                        dialog.accept();
                    }
                },
                Kirigami.Action {
                    text: i18nc("@action:button", "Cancel")
                    icon.name: "dialog-cancel-symbolic"
                    onTriggered: {
                        dialog.reject();
                    }
                    shortcut: StandardKey.Cancel
                }
            ]

            onAccepted: call(adapter.removeDevice(device))

            contentData: [
                Connections {
                    target: dialog.device
                    function onDeviceRemoved(device: BluezQt.Device): void {
                        dialog.reject();
                    }
                },
                Connections {
                    target: dialog.adapter
                    function onAdapterRemoved(adapter: BluezQt.Adapter): void {
                        dialog.reject();
                    }
                    function onPoweredChanged(powered: bool): void {
                        if (!powered) {
                            dialog.reject();
                        }
                    }
                }
            ]
        }
    }

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    function setBluetoothEnabled(enabled: bool): void {
        BluezQt.Manager.bluetoothBlocked = !enabled;

        BluezQt.Manager.adapters.forEach(adapter => {
            adapter.powered = enabled;
        });
    }

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: errorMessage
        position: Kirigami.InlineMessage.Position.Header
        type: Kirigami.MessageType.Error
        showCloseButton: true
    }

    view: ListView {
        id: list
        clip: true

        Kirigami.PlaceholderMessage {
            id: noBluetoothMessage
            // We cannot use the adapter count here because that can be zero when
            // bluetooth is disabled even when there are physical devices
            visible: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown
            icon.name: "edit-none"
            text: i18n("No Bluetooth adapters found")
            explanation: i18n("Connect an external Bluetooth adapter")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        Kirigami.PlaceholderMessage {
            id: bluetoothDisabledMessage
            visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational && !noBluetoothMessage.visible
            icon.name: "network-bluetooth-inactive"
            text: i18n("Bluetooth is disabled")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent

            helpfulAction: Kirigami.Action {
                icon.name: "network-bluetooth-symbolic"
                text: i18n("Enable")
                onTriggered: {
                    root.setBluetoothEnabled(true)
                }
            }
        }

        Kirigami.PlaceholderMessage {
            visible: !noBluetoothMessage.visible && !bluetoothDisabledMessage.visible && list.count === 0
            icon.name: "network-bluetooth-activated"
            text: i18n("No devices paired")
            explanation: xi18nc("@info", "Click <interface>Add New Device…</interface> to pair some")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        model: BluezQt.Manager.bluetoothOperational ? devicesModel : null

        QQC2.BusyIndicator {
            id: busyIndicator
            running: false
            anchors.centerIn: parent
        }


        DevicesProxyModel {
            id: devicesModel
            sourceModel: BluezQt.DevicesModel { }
        }

        section.property: "Connected"
        section.delegate: Kirigami.ListSectionHeader {
            required property string section

            width: ListView.view.width
            text: section === "true" ? i18n("Connected") : i18n("Available")
        }

        delegate: QQC2.ItemDelegate {
            id: delegate

            required property var model

            width: ListView.view.width

            onClicked: root.KCMUtils.ConfigModule.push("Device.qml", { device: model.Device })

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                KD.IconTitleSubtitle {
                    title: delegate.model.Name
                    subtitle: root.infoText(delegate.model.Device)
                    icon.name: delegate.model.Icon
                    icon.width: Kirigami.Units.iconSizes.medium
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    text: delegate.model.Connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: delegate.model.Connected ? "network-disconnect-symbolic" : "network-connect-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered

                    onClicked: {
                        if (delegate.model.Connected) {
                            root.makeCall(delegate.model.Device.disconnectFromDevice())
                        } else {
                            root.makeCall(delegate.model.Device.connectToDevice())
                        }
                    }
                }

                QQC2.ToolButton {
                    text: i18nc("@action:button %1 is the name of a Bluetooth device", "Forget \"%1\"", delegate.model.Name)
                    icon.name: "edit-delete-remove-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered

                    onClicked: {
                        const dialog = forgetDialogComponent.createObject(root, {
                            adapter: delegate.model.Adapter,
                            device: delegate.model.Device,
                            name: delegate.model.Name,
                        });
                        // Use IIFE (Immediately Invoked Function Expression) to hard-copy a reference
                        // to root object, to avoid it being lost if the delegate is destroyed.
                        dialog.call.connect((function (root) {
                            return (call => root.makeCall(call));
                        })(root));
                        dialog.closed.connect(() => dialog.destroy());
                        dialog.open();
                    }
                }
            }
        }
    }

    function infoText(device: BluezQt.Device): string {
        const { battery } = device;
        const labels = [];

        labels.push(Script.deviceTypeToString(device));

        if (battery) {
            labels.push(i18n("%1% Battery", battery.percentage));
        }

        return labels.join(" · ");
    }
}
