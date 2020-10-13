/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QQC2

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2

import org.kde.bluezqt 1.0 as BluezQt

SimpleKCM {

    property var device

    title: device.name

    Connections {
        target: kcm
        function onNetworkAvailable(service, available) {

            if (service === "dun") {
                dunButton.visible = available && device.connected
            }

            if (service === "nap") {
                napButton.visible = available && device.connected
            }
        }
    }

    Connections {
        target: device

        function onConnectedChanged() {
            kcm.checkNetworkConnection(device.uuids, device.address)
        }
    }

    Component.onCompleted: {
        kcm.checkNetworkConnection(device.uuids, device.address)
    }

    header: Kirigami.InlineMessage {
        id: errorMessage
        type: Kirigami.MessageType.Error
        showCloseButton: true
    }

    ColumnLayout {

        Kirigami.Icon {
            source: device.icon
            Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.FormLayout {

            Row {
                QQC2.Button {
                    id: connectButton
                    enabled: !indicator.running
                    text: device.connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: device.connected ? "network-disconnect" : "network-connect"

                    onClicked: {
                        if (device.connected) {
                            makeCall(device.disconnectFromDevice())
                        } else {
                            makeCall(device.connectToDevice())
                        }
                    }

                    function makeCall(call) {
                        indicator.running = true
                        call.finished.connect(call => {
                            indicator.running = false
                            if (call.error) {
                                errorMessage.text = call.errorText
                                errorMessage.visible = true
                            }
                        })
                    }
                }

                QQC2.BusyIndicator {
                    id: indicator
                    running: false
                    height: connectButton.height
                }
            }

            QQC2.Label {
                text: deviceTypeToString(device.type)
                Kirigami.FormData.label: i18n("Type:")
            }

            QQC2.Label {
                text: device.address
                Kirigami.FormData.label: i18n("Address:")
            }

            QQC2.Label {
                text: device.adapter.name
                Kirigami.FormData.label: i18n("Adapter:")
            }

            QQC2.TextField {
                text: device.name
                onTextChanged: device.name = text
                Kirigami.FormData.label: i18n("Name:")
            }

            QQC2.CheckBox {
                text: i18n("Trusted")
                checked: device.trusted
                onClicked: device.trusted = !device.trusted
            }

            QQC2.CheckBox {
                text: i18n("Blocked")
                checked: device.blocked
                onClicked: device.blocked = !device.blocked
            }

            QQC2.Button {
                text: i18n("Send File")
                visible: device.uuids.includes(BluezQt.Services.ObexObjectPush) && device.connected
                onClicked: kcm.runSendFile(device.ubi)
            }

            QQC2.Button {
                id: napButton
                text: i18n("Setup NAP Network...")
                visible: false
                onClicked: kcm.setupNetworkConnection("nap", device.address, device.name)
            }

            QQC2.Button {
                id: dunButton
                text: i18n("Setup DUN Network...")
                visible: false
                onClicked: kcm.setupNetworkConnection("dun", device.address, device.name)
            }
        }
    }

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
        case BluezQt.Device.DeviceAudioVideo:
            return i18nc("This device is an Audio device", "Audio");
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
            return i18nc("This device is an Imaging device (printer, scanner, camera, display, ...)", "Imaging");
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
