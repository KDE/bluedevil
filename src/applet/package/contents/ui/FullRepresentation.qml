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

PlasmaExtras.Representation {

    implicitWidth: PlasmaCore.Units.gridUnit * 24
    implicitHeight: PlasmaCore.Units.gridUnit * 24

    collapseMarginsHint: true

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

    PlasmaComponents3.ScrollView {
        id: scrollView
        anchors.fill: parent

        // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
        PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

        contentItem: ListView {
            id: listView
            readonly property var devicesModel: PlasmaBt.DevicesProxyModel {
                id: devicesModel
                sourceModel: BluezQt.DevicesModel { }
            }
            model: btManager.adapters.length > 0 && !btManager.bluetoothBlocked ? devicesModel : null
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            topMargin: PlasmaCore.Units.smallSpacing * 2
            bottomMargin: PlasmaCore.Units.smallSpacing * 2
            leftMargin: PlasmaCore.Units.smallSpacing * 2
            rightMargin: PlasmaCore.Units.smallSpacing * 2
            spacing: PlasmaCore.Units.smallSpacing

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
                    width: listView.width - PlasmaCore.Units.smallSpacing * 4
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
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: DeviceItem {}

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
}
