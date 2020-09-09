/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.2
import org.kde.plasma.components 2.0 as PlasmaComponents // for ListItem
import org.kde.plasma.components 3.0 as PlasmaComponents3

PlasmaComponents.ListItem {
    id: header

    property alias text: headerLabel.text

    anchors {
        left: parent.left
        right: parent.right
    }

    height: headerLabel.height + units.gridUnit
    sectionDelegate: true

    PlasmaComponents3.Label {
        id: headerLabel
        anchors.centerIn: parent
        font.weight: Font.DemiBold
        elide: Text.ElideRight
    }
}
