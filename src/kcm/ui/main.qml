/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Tom Zander <tom@flowee.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10 as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils

import org.kde.bluezqt 1.0 as BluezQt

import org.kde.plasma.private.bluetooth 1.0

ScrollViewKCM {
    id: root

    actions: [
        Kirigami.Action {
            id: enableAction
            text: i18nc("@action: button as in, 'enable Bluetooth'", "Enabled")
            icon.name: "network-bluetooth-symbolic"
            checkable: true
            checked: BluezQt.Manager.bluetoothOperational
            visible: BluezQt.Manager.rfkill.state !== BluezQt.Rfkill.Unknown
            onTriggered: {
                root.setBluetoothEnabled(!BluezQt.Manager.bluetoothOperational)
            }
            displayComponent: QQC2.Switch {
                text: enableAction.text
                checked: enableAction.checked
                visible: enableAction.visible
                onToggled: enableAction.trigger()
            }
        },
        Kirigami.Action {
            text: i18n("Add New Device…")
            icon.name: "list-add-symbolic"
            onTriggered: kcm.runWizard()
            visible: BluezQt.Manager.bluetoothOperational
        },
        Kirigami.Action {
            text: i18n("Configure…")
            icon.name: "configure-symbolic"
            onTriggered: kcm.push("General.qml")
            visible: BluezQt.Manager.bluetoothOperational
        }
    ]

    function makeCall(call) {
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
        target: kcm

        function onErrorOccured(errorText) {
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
            subtitle: i18n("Are you sure you want to forget \"%1\"?", dialog.name)

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
                    function onDeviceRemoved() {
                        dialog.reject();
                    }
                },
                Connections {
                    target: dialog.adapter
                    function onAdapterRemoved() {
                        dialog.reject();
                    }
                    function onPoweredChanged() {
                        if (!dialog.adapter.powered) {
                            dialog.reject();
                        }
                    }
                }
            ]
        }
    }

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    function setBluetoothEnabled(enabled) {
        BluezQt.Manager.bluetoothBlocked = !enabled

        for (var i = 0; i < BluezQt.Manager.adapters.length; ++i) {
            var adapter = BluezQt.Manager.adapters[i];
            adapter.powered = enabled;
        }
    }

    header: Kirigami.InlineMessage {
        id: errorMessage
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
            icon.name: "edit-none-symbolic"
            text: i18n("No Bluetooth adapters found")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        Kirigami.PlaceholderMessage {
            id: bluetoothDisabledMessage
            visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational && !noBluetoothMessage.visible
            icon.name: "network-bluetooth-inactive-symbolic"
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
            icon.name: "network-bluetooth-activated-symbolic"
            text: i18n("No devices paired")
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
            text: section === "true" ? i18n("Connected") : i18n("Available")
        }

        delegate: Kirigami.SwipeListItem {

            // content item includes its own padding
            padding: 0

            contentItem: Kirigami.BasicListItem {
                // The parent item already has a highlight
                activeBackgroundColor: "transparent"

                separatorVisible: false

                text: model.Name
                icon.name: model.Icon
                iconSize: Kirigami.Units.iconSizes.medium
                onClicked: kcm.push("Device.qml", {device: model.Device})
            }

            actions: [
                Kirigami.Action {
                    text: model.Connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: model.Connected ? "network-disconnect-symbolic" : "network-connect-symbolic"
                    onTriggered: {
                        if (model.Connected) {
                            root.makeCall(model.Device.disconnectFromDevice())
                        } else {
                            root.makeCall(model.Device.connectToDevice())
                        }
                    }
                },
                Kirigami.Action {
                    text: i18nc("@action:button %1 is the name of a Bluetooth device", "Forget \"%1\"", model.Name)
                    icon.name: "edit-delete-remove-symbolic"
                    onTriggered: {
                        const dialog = forgetDialogComponent.createObject(root, {
                            adapter: model.Adapter,
                            device: model.Device,
                            name: model.Name,
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
            ]
        }
    }

    // System Settings doesn't draw its own footer buttons, so extra footer
    // paddings here to make them look better isn't necessary
    extraFooterTopPadding: false
}
