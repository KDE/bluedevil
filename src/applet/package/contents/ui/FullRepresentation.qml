/*
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>
    Copyright 2014-2015 David Rosca <nowrep@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.4
import QtQuick.Controls 2.4
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

PlasmaComponents3.Page {

    Action {
        id: addBluetoothDeviceAction

        enabled: btManager.devices.length === 0

        text: i18n("Add New Device")
        icon.name: "list-add"

        onTriggered: {
            PlasmaBt.LaunchApp.runCommand("bluedevil-wizard");
        }
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
        enabled: !btManager.bluetoothBlocked
    }

    FocusScope {
        anchors.fill: parent
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
                anchors.fill: parent
                clip: true
                model: devicesModel
                currentIndex: -1
                enabled: btManager.bluetoothOperational
                boundsBehavior: Flickable.StopAtBounds
                section.property: "Section"
                section.delegate: Header {
                    text: section == "Connected" ? i18n("Connected devices") : i18n("Available devices")
                }
                highlight: PlasmaComponents.Highlight { }
                highlightMoveDuration: units.longDuration
                highlightResizeDuration: units.longDuration
                delegate: DeviceItem { }
            }
        }

        // Not inside the ListView because we want the listview to be hidden
        // when Bluetooth is disabled, yet still show an "Enable Bluetooth"
        // message
        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: units.largeSpacing

            visible: text.length > 0

            text: {
                if (btManager.adapters.length === 0) {
                    return i18n("No Bluetooth Adapters Available")
                } else if (btManager.bluetoothBlocked) {
                    return i18n("Bluetooth is Disabled")
                } else if (btManager.devices.length === 0) {
                    return i18n("No Devices Found")
                }
                return ""
            }

            helpfulAction: {
                if (btManager.bluetoothBlocked) {
                    return enableBluetoothAction
                } else if (btManager.devices.length === 0) {
                    return addBluetoothDeviceAction
                }
                return undefined
            }
        }
    }
}
