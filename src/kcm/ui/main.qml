/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Tom Zander <tom@flowee.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils

import org.kde.bluezqt 1.0 as BluezQt

import org.kde.plasma.private.bluetooth 1.0

ScrollViewKCM {
    id: root

    actions: [
        Kirigami.Action {
            id: enableAction
            text: i18nc("@action: button", "Enable Bluetooth")
            icon.name: "network-bluetooth"
            checkable: true
            checked: BluezQt.Manager.bluetoothOperational
            onTriggered: {
                root.setBluetoothEnabled(!BluezQt.Manager.bluetoothOperational)
            }
            displayComponent: QQC2.Switch {
                text: enableAction.text
                checked: enableAction.checked
                onToggled: enableAction.trigger()
            }
        },
        Kirigami.Action {
            id: addAction
            text: i18nc("@action:button", "Add New Device…")
            icon.name: "list-add"
            onTriggered: kcm.runWizard()
            enabled: BluezQt.Manager.bluetoothOperational
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
                    icon.name: "edit-delete-remove"
                    onTriggered: {
                        dialog.accept();
                    }
                },
                Kirigami.Action {
                    text: i18nc("@action:button", "Cancel")
                    icon.name: "dialog-cancel"
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
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        Kirigami.PlaceholderMessage {
            id: bluetoothDisabledMessage
            visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational && !noBluetoothMessage.visible
            icon.name: "network-bluetooth"
            text: i18n("Bluetooth is disabled")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent

            helpfulAction: enableAction
        }

        Kirigami.PlaceholderMessage {
            visible: !noBluetoothMessage.visible && !bluetoothDisabledMessage.visible && list.count === 0
            icon.name: "network-bluetooth-activated"
            text: i18n("No devices paired")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent

            helpfulAction: addAction
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
            text: section === "true" ? i18n("Connected devices") : i18n("Paired devices")
        }

        delegate: Kirigami.AbstractListItem {
            // There's no need for a list item to ever be selected
            down: false
            highlighted: false
            hoverEnabled: false
            // ... and because of that, use alternating backgrounds to visually
            // connect list items' left and right side content elements
            alternatingBackground: true

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.Medium
                    Layout.preferredWidth: Layout.preferredHeight
                    source: model.Icon
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    spacing: 0

                    Kirigami.Heading {
                        Layout.fillWidth: true

                        text: model.Name
                        level: 5
                        elide: Text.ElideRight
                    }

                    QQC2.Label {
                        Layout.fillWidth: true

                        text: deviceTypeToString(model.Device.type)
                        visible: text.length > 0
                        font: Kirigami.Theme.smallestFont
                        opacity: color === Kirigami.Theme.textColor ? 0.7 : 1.0
                        elide: Text.ElideRight
                    }
                }

                /* TODO: Show on connecting(/disconnecting?)
                QQC2.BusyIndicator {
                    id: indicator
                    running: false
                    height: connectButton.height
                }
                */

                QQC2.ToolButton {
                    // TODO: Disable when connecting
                    text: i18nc("@action:button", "Configure…")
                    icon.name: "configure"
                    onClicked: kcm.push("Device.qml", {device: model.Device})
                }

                QQC2.ToolButton {
                    text: model.Connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: model.Connected ? "network-disconnect" : "network-connect"
                    onClicked: {
                        if (model.Connected) {
                            root.makeCall(model.Device.disconnectFromDevice())
                        } else {
                            root.makeCall(model.Device.connectToDevice())
                        }
                    }
                }

                // TODO: Rather than makeCall and use busyIndicator, show it on the device itself (per-device)
                // Applet does this!!!

                QQC2.ToolButton {
                    icon.name: "edit-delete-remove"

                    onClicked: {
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

                    QQC2.ToolTip {
                        text: i18nc("@info:tooltip %1 is the name of a Bluetooth device", "Forget \"%1\"", model.Name)
                    }
                }
            }
        }
    }

    footer: ColumnLayout {
        id: footerLayout

        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            showCloseButton: true

            // TODO: Set a real message, rather than showing raw error strings
            // i.e. add a function that sets the correct text for an error, falling back to the raw error message
        }

        Kirigami.FormLayout {
            id: form

            property QtObject adapter: BluezQt.Manager.adapters[adaptersBox.currentIndex]

            QQC2.ComboBox {
                id: adaptersBox
                Kirigami.FormData.label: i18n("Device:")
                model: BluezQt.Manager.adapters
                textRole: "name"
                visible: count > 1
            }

            QQC2.TextField {
                text: form.adapter.name
                Kirigami.FormData.label: i18n("Name:")
                onEditingFinished: form.adapter.name = text
            }

            QQC2.Label {
                text: form.adapter.address
                Kirigami.FormData.label: i18n("Address:")
            }

            QQC2.CheckBox {
                Kirigami.FormData.label: i18n("Enabled:")
                checked: form.adapter.powered
                onToggled: form.adapter.powered = checked
                visible: adaptersBox.count > 1
            }

            QQC2.CheckBox {
                Kirigami.FormData.label: i18n("Visible:")
                checked: form.adapter.discoverable
                onToggled: form.adapter.discoverable = checked
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
            }

            QQC2.Button {
                Kirigami.FormData.label: i18n("Behavior:")
                text: i18nc("@action:button", "Configure…")
                icon.name: "preferences-system-bluetooth"
                onClicked: kcm.push("General.qml")
            }
        }
    }

    // System Settings doesn't draw its own footer buttons, so extra footer
    // paddings here to make them look better isn't necessary
    extraFooterTopPadding: false

    // TODO: Duplicated in Device
    function deviceTypeToString(type) {
        switch (type) {
        case BluezQt.Device.Phone:
            return i18nc("This device is a Phone", "Phone");
        case BluezQt.Device.Modem:
            return i18nc("This device is a Modem", "Modem");
        case BluezQt.Device.Computer:
            return i18nc("This device is a Computer", "Computer");
        case BluezQt.Device.Network:
            return i18nc("This device is of type Network", "Network");
        case BluezQt.Device.Headset:
            return i18nc("This device is a Headset", "Headset");
        case BluezQt.Device.Headphones:
            return i18nc("This device is a Headphones", "Headphones");
        case BluezQt.Device.AudioVideo:
            return i18nc("This device is an Audio/Video device", "Multimedia Device");
        case BluezQt.Device.Keyboard:
            return i18nc("This device is a Keyboard", "Keyboard");
        case BluezQt.Device.Mouse:
            return i18nc("This device is a Mouse", "Mouse");
        case BluezQt.Device.Joypad:
            return i18nc("This device is a Joypad", "Joypad");
        case BluezQt.Device.Tablet:
            return i18nc("This device is a Graphics Tablet (input device)", "Tablet");
        case BluezQt.Device.Peripheral:
            return i18nc("This device is a Peripheral device", "Peripheral");
        case BluezQt.Device.Camera:
            return i18nc("This device is a Camera", "Camera");
        case BluezQt.Device.Printer:
            return i18nc("This device is a Printer", "Printer");
        case BluezQt.Device.Imaging:
            return i18nc("This device is an Imaging device (printer, scanner, camera, display, …)", "Imaging");
        case BluezQt.Device.Wearable:
            return i18nc("This device is a Wearable", "Wearable");
        case BluezQt.Device.Toy:
            return i18nc("This device is a Toy", "Toy");
        case BluezQt.Device.Health:
            return i18nc("This device is a Health device", "Health");
        default:
            return i18nc("Type of device: could not be determined", "Unknown");
        }
    }
}
