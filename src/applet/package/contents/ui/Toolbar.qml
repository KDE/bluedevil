/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

import org.kde.kquickcontrolsaddons 2.0

PlasmaExtras.PlasmoidHeading {
    id: toolbar

    property alias onSwitch: onSwitch

    leftPadding: mirrored ? 0 : Kirigami.Units.smallSpacing
    rightPadding: mirrored ? Kirigami.Units.smallSpacing : 0

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        PlasmaComponents3.Switch {
            id: onSwitch
            text: i18n("Enable Bluetooth")
            icon.name: "preferences-system-bluetooth"
            checked: btManager.bluetoothOperational
            enabled: btManager.bluetoothBlocked || btManager.adapters.length > 0
            focus: bluetoothApplet.expanded
            onToggled: toggleBluetooth()
        }

        Item {
            Layout.fillWidth: true
        }

        PlasmaComponents3.ToolButton {
            id: addDeviceButton

            property QtObject /*QAction*/ qAction: bluetoothApplet.addDeviceAction

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

            property QtObject /*QAction*/ qAction: Plasmoid.internalAction("configure")

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
