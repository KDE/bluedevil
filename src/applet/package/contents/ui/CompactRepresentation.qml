/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

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
