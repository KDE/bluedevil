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

import QtQuick 2.2
import QtQuick.Layouts 1.2
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

PlasmaExtras.PlasmoidHeading {
    id: toolbar

    RowLayout {
        anchors.fill: parent
        spacing: units.smallSpacing

        SwitchButton {
            checked: btManager.bluetoothOperational
            enabled: btManager.bluetoothBlocked || btManager.adapters.length
            icon: "preferences-system-bluetooth"
            tooltip: i18n("Enable Bluetooth")

            onClicked: toggleBluetooth()
        }

        Item {
            Layout.fillWidth: true
        }

        PlasmaComponents3.ToolButton {
            id: addDeviceButton

            icon.name: "list-add"

            onClicked: {
                PlasmaBt.LaunchApp.runCommand("bluedevil-wizard");
            }

            PlasmaComponents3.ToolTip {
                text: i18n("Add New Device...")
            }
        }

        PlasmaComponents3.ToolButton {
            id: openSettingsButton

            icon.name: "configure"

            onClicked: {
                KCMShell.open(["bluedevildevices", "bluedeviladapters", "bluedevilglobal"]);
            }

            PlasmaComponents3.ToolTip {
                text: i18n("Configure Bluetooth...")
            }
        }
    }

    function toggleBluetooth()
    {
        var enable = !btManager.bluetoothOperational;
        btManager.bluetoothBlocked = !enable;

        for (var i = 0; i < btManager.adapters.length; ++i) {
            var adapter = btManager.adapters[i];
            adapter.powered = enable;
        }
    }
}
