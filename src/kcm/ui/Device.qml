/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils

import org.kde.bluezqt as BluezQt

import "script.js" as Script

KCMUtils.SimpleKCM {

    required property BluezQt.Device device

    title: device.name

    Connections {
        target: kcm
        function onNetworkAvailable(service: string, available: bool): void {
            switch (service) {
            case "dun":
                dunButton.visible = available && device.connected;
                break;
            case "nap":
                napButton.visible = available && device.connected;
                break;
            default:
                break;
            }
        }
    }

    Connections {
        target: device

        function onConnectedChanged(connected: bool): void {
            kcm.checkNetworkConnection(device.uuids, device.address)
        }
    }

    Component.onCompleted: {
        kcm.checkNetworkConnection(device.uuids, device.address)
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
            source: device.icon
            Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.FormLayout {

            Row {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    id: connectButton
                    enabled: !indicator.running
                    text: device.connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: device.connected ? "network-disconnect-symbolic" : "network-connect-symbolic"

                    onClicked: {
                        if (device.connected) {
                            makeCall(device.disconnectFromDevice())
                        } else {
                            makeCall(device.connectToDevice())
                        }
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
                }

                QQC2.BusyIndicator {
                    id: indicator
                    running: false
                    height: connectButton.height
                }
            }

            QQC2.Label {
                text: Script.deviceTypeToString(device)
                Kirigami.FormData.label: i18n("Type:")
            }

            QQC2.Label {
                text: device.battery !== null ? i18n("%1%", device.battery.percentage) : ""
                visible: device.battery !== null
                Kirigami.FormData.label: i18n("Battery:")
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
                onTextEdited: device.name = text
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
                text: i18n("Setup NAP Network…")
                visible: false
                onClicked: kcm.setupNetworkConnection("nap", device.address, device.name)
            }

            QQC2.Button {
                id: dunButton
                text: i18n("Setup DUN Network…")
                visible: false
                onClicked: kcm.setupNetworkConnection("dun", device.address, device.name)
            }
        }
    }
}
