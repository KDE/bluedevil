/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

import org.kde.bluezqt 1.0 as BluezQt

PlasmaExtras.Representation {
    id: root

    readonly property bool emptyList: btManager.devices.length === 0

    implicitWidth: PlasmaCore.Units.gridUnit * 24
    implicitHeight: PlasmaCore.Units.gridUnit * 24

    focus: true
    collapseMarginsHint: true

    Keys.onDownPressed: {
        if (listView.count === 0) {
            return;
        }
        if (listView.currentIndex < 0 || toolbar.checkbox.activeFocus) {
            listView.incrementCurrentIndex();
            listView.currentItem.forceActiveFocus();
        } else {
            event.accepted = false;
        }
    }

    Action {
        id: addBluetoothDeviceAction

        enabled: root.emptyList

        text: Plasmoid.action("addNewDevice").text
        icon.name: "list-add"

        onTriggered: Plasmoid.action("addNewDevice").trigger()
    }

    Action {
        id: enableBluetoothAction

        enabled: btManager.bluetoothBlocked

        text: i18n("Enable")
        icon.name: "preferences-system-bluetooth"

        onTriggered: bluetoothApplet.toggleBluetooth()
    }

    header: Toolbar {
        id: toolbar
        visible: btManager.adapters.length > 0
        focus: true
    }

    PlasmaComponents3.ScrollView {
        id: scrollView
        anchors.fill: parent

        // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
        PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

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
                active: section !== "Connected" && bluetoothApplet.connectedDevices.length > 0
                // Need to manually set the height or else the loader takes up
                // space after the first time it unloads a previously-loaded item
                height: active ? PlasmaCore.Units.gridUnit : 0

                // give us 2 frames to try and figure out a layout, this reduces jumpyness quite a bit but doesn't
                // entirely eliminate it https://bugs.kde.org/show_bug.cgi?id=438610
                Behavior on height { PropertyAnimation { duration: 32 } }

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
            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: DeviceItem {}

            Keys.onUpPressed: {
                if (listView.currentIndex === 0) {
                    listView.currentIndex = -1;
                    toolbar.checkbox.forceActiveFocus(Qt.BacktabFocusReason);
                } else {
                    event.accepted = false;
                }
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (4 * PlasmaCore.Units.largeSpacing)
                active: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || btManager.bluetoothBlocked || root.emptyList
                sourceComponent: PlasmaExtras.PlaceholderMessage {
                    iconName: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || btManager.bluetoothBlocked ? "network-bluetooth" : "edit-none"

                    text: {
                        // We cannot use the adapter count here because that can be zero when
                        // bluetooth is disabled even when there are physical devices
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return i18n("No Bluetooth adapters available");
                        } else if (btManager.bluetoothBlocked) {
                            return i18n("Bluetooth is disabled");
                        } else if (root.emptyList) {
                            return i18n("No devices found");
                        } else {
                            return "";
                        }
                    }

                    helpfulAction: {
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return null;
                        } else if (btManager.bluetoothBlocked) {
                            return enableBluetoothAction;
                        } else if (root.emptyList) {
                            return addBluetoothDeviceAction;
                        } else {
                            return null;
                        }
                    }
                }
            }
        }
    }
}
