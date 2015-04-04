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
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.bluetooth 1.0 as PlasmaBt

PlasmaComponents.ListItem {
    id: deviceItem

    property bool expanded : visibleDetails
    property bool visibleDetails : false
    property bool connecting : false
    property int baseHeight : deviceItemBase.height
    property var currentDeviceDetails : []

    height: expanded ? baseHeight + expandableComponentLoader.height + Math.round(units.gridUnit / 3) : baseHeight
    checked: ListView.isCurrentItem
    enabled: true

    Item {
        id: deviceItemBase

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            // Reset top margin from PlasmaComponents.ListItem
            topMargin: -Math.round(units.gridUnit / 3)
        }

        height: Math.max(units.iconSizes.medium, deviceNameLabel.height + deviceInfoLabel.height) + Math.round(units.gridUnit / 2)

        PlasmaCore.IconItem {
            id: deviceIcon

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            height: units.iconSizes.medium
            width: height
            source: Icon
        }

        PlasmaComponents.Label {
            id: deviceNameLabel

            anchors {
                bottom: deviceIcon.verticalCenter
                left: deviceIcon.right
                leftMargin: Math.round(units.gridUnit / 2)
                right: connectButton.visible ? connectButton.left : parent.right
            }

            height: paintedHeight
            elide: Text.ElideRight
            font.weight: Connected ? Font.DemiBold : Font.Normal
            font.italic: connecting
            text: DeviceFullName
            textFormat: Text.PlainText
        }

        PlasmaComponents.Label {
            id: deviceInfoLabel

            anchors {
                left: deviceIcon.right
                leftMargin: Math.round(units.gridUnit / 2)
                right: connectButton.visible ? connectButton.left : parent.right
                top: deviceNameLabel.bottom
            }

            height: paintedHeight
            elide: Text.ElideRight
            font.pointSize: theme.smallestFont.pointSize
            opacity: 0.6
            text: infoText()
            textFormat: Text.PlainText
        }

        PlasmaComponents.BusyIndicator {
            id: connectingIndicator

            anchors {
                right: parent.right
                rightMargin: Math.round(units.gridUnit / 2)
                verticalCenter: deviceIcon.verticalCenter
            }

            height: units.iconSizes.medium
            width: height
            running: connecting
            visible: running && !connectButton.visible
        }

        PlasmaComponents.Button {
            id: connectButton

            anchors {
                right: parent.right
                rightMargin: Math.round(units.gridUnit / 2)
                verticalCenter: deviceIcon.verticalCenter
            }

            text: Connected ? i18n("Disconnect") : i18n("Connect")
            opacity: !connecting && deviceItem.containsMouse ? 1 : 0
            visible: opacity != 0

            Behavior on opacity {
                NumberAnimation {
                    duration: units.shortDuration
                }
            }

            onClicked: connectToDevice()
        }
    }

    Loader {
        id: expandableComponentLoader

        anchors {
            left: parent.left
            right: parent.right
            top: deviceItemBase.bottom
        }
    }

    Component {
        id: detailsComponent

        Item {
            height: childrenRect.height

            PlasmaCore.SvgItem {
                id: detailsSeparator

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                height: lineSvg.elementSize("horizontal-line").height
                width: parent.width
                elementId: "horizontal-line"
                svg: PlasmaCore.Svg {
                    id: lineSvg
                    imagePath: "widgets/line"
                }
            }

            Column {
                id: actionsColumn

                anchors {
                    left: parent.left
                    leftMargin: units.iconSizes.medium
                    right: parent.right
                    top: detailsSeparator.bottom
                }

                PlasmaComponents.ToolButton {
                    id: browseFilesButton
                    text: i18n("Browse Files")
                    iconSource: "folder"
                    visible: Uuids.indexOf(BluezQt.Services.ObexFileTransfer) != -1

                    onClicked: {
                        var url = "obexftp://%1".arg(Address.replace(/:/g, "-"));
                        PlasmaBt.LaunchApp.runUrl(url, "inode/directory");
                    }
                }

                PlasmaComponents.ToolButton {
                    id: sendFileButton
                    text: i18n("Send File")
                    iconSource: "folder-download"
                    visible: Uuids.indexOf(BluezQt.Services.ObexObjectPush) != -1

                    onClicked: {
                        PlasmaBt.LaunchApp.runCommand("bluedevil-sendfile", ["-u", Ubi]);
                    }
                }
            }

            PlasmaCore.SvgItem {
                id: actionsSeparator

                anchors {
                    left: parent.left
                    right: parent.right
                    top: actionsColumn.bottom
                }

                visible: browseFilesButton.visible || sendFileButton.visible
                height: lineSvg.elementSize("horizontal-line").height
                width: parent.width
                elementId: "horizontal-line"
                svg: lineSvg
            }

            Column {
                id: details

                anchors {
                    left: parent.left
                    leftMargin: units.iconSizes.medium
                    right: parent.right
                    top: actionsSeparator.bottom
                    topMargin: Math.round(units.gridUnit / 3)
                }

                Repeater {
                    id: repeater

                    property int longestString: 0

                    model: currentDeviceDetails.length / 2

                    Item {
                        anchors {
                            left: parent.left
                            right: parent.right
                            topMargin: Math.round(units.gridUnit / 3)
                        }

                        height: Math.max(detailNameLabel.height, detailValueLabel.height)

                        PlasmaComponents.Label {
                            id: detailNameLabel

                            anchors {
                                left: parent.left
                                leftMargin: repeater.longestString - paintedWidth + Math.round(units.gridUnit / 2)
                                verticalCenter: parent.verticalCenter
                            }

                            height: paintedHeight
                            font.pointSize: theme.smallestFont.pointSize
                            horizontalAlignment: Text.AlignRight
                            opacity: 0.6
                            text: "<b>%1</b>: &nbsp".arg(currentDeviceDetails[index * 2])

                            Component.onCompleted: {
                                if (paintedWidth > repeater.longestString) {
                                    repeater.longestString = paintedWidth;
                                }
                            }
                        }

                        PlasmaComponents.Label {
                            id: detailValueLabel;

                            anchors {
                                left: detailNameLabel.right
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                            }

                            height: paintedHeight
                            elide: Text.ElideRight
                            font.pointSize: theme.smallestFont.pointSize
                            opacity: 0.6
                            text: currentDeviceDetails[(index*2)+1]
                            textFormat: Text.StyledText
                        }
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "collapsed"
            when: !visibleDetails

            StateChangeScript {
                script: if (expandableComponentLoader.status == Loader.Ready) {
                            expandableComponentLoader.sourceComponent = undefined;
                        }
            }
        },

        State {
            name: "expandedDetails"
            when: visibleDetails

            StateChangeScript {
                script: createContent();
            }
        }
    ]

    onStateChanged: {
        if (state == "expandedDetails") {
            ListView.view.currentIndex = index;
        }
    }

    onClicked: {
        visibleDetails = !visibleDetails;

        if (!visibleDetails) {
            ListView.view.currentIndex = -1;
        }
    }

    function boolToString(v)
    {
        if (v) {
            return i18n("Yes");
        }
        return i18n("No");
    }

    function adapterName(a)
    {
        var hci = devicesModel.adapterHciString(a.ubi);
        if (hci != "") {
            return "%1 (%2)".arg(a.name).arg(hci);
        }
        return a.name;
    }

    function createContent() {
        if (visibleDetails) {
            var details = [];

            if (Name != RemoteName) {
                details.push(i18n("Remote Name"));
                details.push(RemoteName);
            }

            details.push(i18n("Address"));
            details.push(Address);

            details.push(i18n("Paired"));
            details.push(boolToString(Paired));

            details.push(i18n("Trusted"));
            details.push(boolToString(Trusted));

            details.push(i18n("Adapter"));
            details.push(adapterName(Adapter));

            currentDeviceDetails = details;
            expandableComponentLoader.sourceComponent = detailsComponent;
        }
    }

    function infoText()
    {
        if (connecting) {
            return i18n("Connecting");
        }

        switch (DeviceType) {
        case BluezQt.Device.Headset:
        case BluezQt.Device.Headphones:
        case BluezQt.Device.OtherAudio:
            return i18n("Audio device");

        case BluezQt.Device.Keyboard:
        case BluezQt.Device.Mouse:
        case BluezQt.Device.Joypad:
        case BluezQt.Device.Tablet:
            return i18n("Input device");

        default:
            break;
        }

        var profiles = [];

        if (Uuids.indexOf(BluezQt.Services.ObexFileTransfer) != -1) {
            profiles.push(i18n("File transfer"));
        }
        if (Uuids.indexOf(BluezQt.Services.ObexObjectPush) != -1) {
            profiles.push(i18n("Send file"));
        }
        if (Uuids.indexOf(BluezQt.Services.HumanInterfaceDevice) != -1) {
            profiles.push(i18n("Input"));
        }
        if (Uuids.indexOf(BluezQt.Services.AdvancedAudioDistribution) != -1) {
            profiles.push(i18n("Audio"));
        }
        if (Uuids.indexOf(BluezQt.Services.Nap) != -1) {
            profiles.push(i18n("Network"));
        }

        if (profiles.length == 0) {
            return i18n("Other device");
        }

        return profiles.join(", ");
    }

    function connectToDevice()
    {
        if (Connected) {
            Device.disconnectDevice();
            return;
        }

        if (connecting) {
            return;
        }

        connecting = true;
        runningActions++;

        Device.connectDevice().finished.connect(function(call) {
            connecting = false;
            runningActions--;

            if (call.error) {
                var text = "";
                var title = "%1 (%2)".arg(Name).arg(Address);

                switch (call.error) {
                case BluezQt.PendingCall.Failed:
                    if (call.errorText == "Host is down") {
                        text = i18nc("Notification when the connection failed due to Failed:HostIsDown",
                                     "The device is unreachable");
                    } else {
                        text = i18nc("Notification when the connection failed due to Failed",
                                     "Connection to the device failed");
                    }
                    break;

                case BluezQt.PendingCall.NotReady:
                    text = i18nc("Notification when the connection failed due to NotReady",
                                 "The device is not ready");
                    break;

                default:
                    return;
                }

                PlasmaBt.Notify.connectionFailed(title, text);
            }
        });
    }
}
