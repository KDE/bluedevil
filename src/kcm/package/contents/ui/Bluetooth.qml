/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10 as QQC2

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcm 1.2

import org.kde.bluezqt 1.0 as BluezQt

import org.kde.plasma.private.bluetooth 1.0

ScrollViewKCM {

    id: root

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
            text: i18n("No Bluetooth adapters found")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        Kirigami.PlaceholderMessage {
            id: bluetoothDisabledMessage
            visible: BluezQt.Manager.operational && !BluezQt.Manager.bluetoothOperational && !noBluetoothMessage.visible
            text: i18n("Bluetooth is disabled")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent

            helpfulAction: Kirigami.Action {
                iconName: "network-bluetooth"
                text: i18n("Enable")
                onTriggered: {
                    root.setBluetoothEnabled(true)
                }
            }
        }

        Kirigami.PlaceholderMessage {
            visible: !noBluetoothMessage.visible && !bluetoothDisabledMessage.visible && list.count === 0
            text: i18n("No devices paired")
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            anchors.centerIn: parent
        }

        model: BluezQt.Manager.bluetoothOperational ? devicesModel : []

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
                icon: model.Icon
                iconSize: Kirigami.Units.iconSizes.medium
                onClicked: kcm.push("Device.qml", {device: model.Device})
            }

            actions: [
                Kirigami.Action {
                    text: model.Connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: model.Connected ? "network-disconnect" : "network-connect"
                    onTriggered: {
                        if (model.Connected) {
                            makeCall(model.Device.disconnectFromDevice())
                        } else {
                            makeCall(model.Device.connectToDevice())
                        }
                    }
                },
                Kirigami.Action {
                    text: i18n("Remove")
                    icon.name: "list-remove"
                    onTriggered: {
                        makeCall(model.Adapter.removeDevice(model.Device))
                    }
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
        }
    }

    footer: RowLayout {

        QQC2.Button {
            text: i18n("Add...")
            visible: BluezQt.Manager.bluetoothOperational
            icon.name: "list-add"
            onClicked: kcm.runWizard()
        }

        QQC2.Button {
            visible: BluezQt.Manager.bluetoothOperational
            text: i18n("Disable Bluetooth")
            icon.name: "network-bluetooth"
            onClicked: {
                root.setBluetoothEnabled(false)
            }
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.Button {
            visible: BluezQt.Manager.bluetoothOperational
            text: i18n("Configure...")
            icon.name: "configure"
            onClicked: kcm.push("General.qml")
        }
    }
}
