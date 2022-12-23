/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

import org.kde.kquickcontrolsaddons 2.0

PlasmaExtras.PlasmoidHeading {
    id: toolbar

    property alias checkbox: checkbox

    leftPadding: PlasmaCore.Units.smallSpacing
    contentItem: RowLayout {
        spacing: PlasmaCore.Units.smallSpacing

        PlasmaComponents3.CheckBox {
            id: checkbox
            text: i18n("Enable Bluetooth")
            icon.name: "preferences-system-bluetooth"
            checked: btManager.bluetoothOperational
            enabled: btManager.bluetoothBlocked || btManager.adapters.length > 0
            focus: Plasmoid.expanded
            onToggled: toggleBluetooth()
        }

        Item {
            Layout.fillWidth: true
        }

        PlasmaComponents3.ToolButton {
            id: addDeviceButton

            property QtObject /*QAction*/ qAction: Plasmoid.action("addNewDevice")

            visible: !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            enabled: qAction.visible

            icon.name: "list-add"

            onClicked: qAction.trigger()

            PlasmaComponents3.ToolTip {
                text: addDeviceButton.qAction.text
            }
            Accessible.name: qAction.text
        }

        PlasmaComponents3.ToolButton {
            id: openSettingsButton

            property QtObject /*QAction*/ qAction: Plasmoid.action("configure")

            visible: !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            icon.name: "configure"
            onClicked: qAction.trigger()

            PlasmaComponents3.ToolTip {
                text: openSettingsButton.qAction.text
            }
            Accessible.name: qAction.text
        }
    }
}
