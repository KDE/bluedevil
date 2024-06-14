/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

MouseArea {
    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton

    property bool wasExpanded

    onPressed: mouse => {
        wasExpanded = bluetoothApplet.expanded
    }

    onClicked: mouse => {
        if (mouse.button === Qt.MiddleButton) {
            toggleBluetooth();
        } else {
            bluetoothApplet.expanded = !wasExpanded;
        }
    }

    hoverEnabled: true

    Kirigami.Icon {
        id: bluetoothIcon
        anchors.fill: parent
        source: Plasmoid.icon
        active: parent.containsMouse
    }
}
