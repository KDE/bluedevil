/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.4
import QtQuick.Controls 2.4
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

import "logic.js" as Logic

PlasmaComponents3.Page {

    Action {
        id: addBluetoothDeviceAction

        enabled: btManager.devices.length === 0

        text: plasmoid.action("addNewDevice").text
        icon.name: "list-add"

        onTriggered: plasmoid.action("addNewDevice").trigger()
    }
    Action {
        id: enableBluetoothAction

        enabled: btManager.bluetoothBlocked

        text: i18n("Enable Bluetooth")
        icon.name: "preferences-system-bluetooth"

        onTriggered: {
            toolbar.toggleBluetooth();
        }
    }

    header: Toolbar {
        id: toolbar
        visible: btManager.adapters.length > 0
    }

    FocusScope {
        anchors.fill: parent
        anchors.topMargin: PlasmaCore.Units.smallSpacing

        focus: true

        PlasmaBt.DevicesProxyModel {
            id: devicesModel
            sourceModel: BluezQt.DevicesModel { }
        }

        PlasmaExtras.ScrollArea {
            id: scrollView

            anchors.fill: parent

            visible: btManager.adapters.length > 0 && !btManager.bluetoothBlocked

            ListView {
                id: listView
                anchors.fill: parent
                clip: true
                model: devicesModel
                currentIndex: -1
                enabled: btManager.bluetoothOperational
                boundsBehavior: Flickable.StopAtBounds
                section.property: "Section"
                // We want to hide the section delegate for the "Connected"
                // group because it's unnecessary; all we want to do here is
                // separate the connected devices from the available ones
                section.delegate: Loader {
                    active: section != "Connected" && Logic.conectedDevicesCount() > 0
                    // Need to manually set the height or else the loader takes up
                    // space after the first time it unloads a previously-loaded item
                    height: active ? PlasmaCore.Units.gridUnit : 0

                    sourceComponent: Item {
                        width: listView.width
                        height: PlasmaCore.Units.gridUnit

                        PlasmaCore.SvgItem {
                            width: parent.width - PlasmaCore.Units.gridUnit * 2
                            anchors.centerIn: parent
                            id: separatorLine
                            svg: PlasmaCore.Svg {
                                imagePath: "widgets/line"
                            }
                            elementId: "horizontal-line"
                        }
                    }
                }
                highlight: PlasmaComponents.Highlight { }
                highlightMoveDuration: PlasmaCore.Units.longDuration
                highlightResizeDuration: PlasmaCore.Units.longDuration
                delegate: DeviceItem {
                    width: listView.width
                }
            }
        }

        // Not inside the ListView because we want the listview to be hidden
        // when Bluetooth is disabled, yet still show an "Enable Bluetooth"
        // message
        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: PlasmaCore.Units.largeSpacing

            visible: text.length > 0

            text: {
                // We cannot use the adapter count here because that can be zero when
                // bluetooth is disabled even when there are physical devices
                if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                    return i18n("No Bluetooth Adapters Available")
                } else if (btManager.bluetoothBlocked) {
                    return i18n("Bluetooth is Disabled")
                } else if (btManager.devices.length === 0) {
                    return i18n("No Devices Found")
                }
                return ""
            }

            helpfulAction: {
                if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                    return null
                } else if (btManager.bluetoothBlocked) {
                    return enableBluetoothAction
                } else if (btManager.devices.length === 0) {
                    return addBluetoothDeviceAction
                }
                return null
            }
        }
    }
}
