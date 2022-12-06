/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

MouseArea {
    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton

    property bool wasExpanded

    onPressed: wasExpanded = Plasmoid.expanded
    onClicked: {
        if (mouse.button === Qt.MiddleButton) {
            toggleBluetooth();
        } else {
            Plasmoid.expanded = !wasExpanded;
        }
    }

    hoverEnabled: true

    PlasmaCore.IconItem {
        id: bluetoothIcon
        anchors.fill: parent
        source: Plasmoid.icon
        active: parent.containsMouse
    }
}
