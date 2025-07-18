/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.bluezqt as BluezQt
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.bluetooth as PlasmaBt

PlasmaExtras.Representation {
    id: root

    required property PlasmoidItem plasmoidItem
    required property PlasmaCore.Action addDeviceAction
    required property PlasmaCore.Action toggleBluetoothAction
    required property PlasmaCore.Action toggleBadgeAction
    required property PlasmaCore.Action configureAction

    readonly property bool emptyList: BluezQt.Manager.devices.length === 0

    implicitWidth: Kirigami.Units.gridUnit * 24
    implicitHeight: Kirigami.Units.gridUnit * 24

    focus: true
    collapseMarginsHint: true

    PlasmaBt.DevicesProxyModel {
        id: devicesModel
        hideBlockedDevices: true
        sourceModel: PlasmaBt.SharedDevicesStateProxyModel
    }

    Keys.onDownPressed: event => {
        if (listView.count === 0) {
            return;
        }
        if (listView.currentIndex < 0 || toolbar.onSwitch.activeFocus) {
            listView.incrementCurrentIndex();
            listView.currentItem.forceActiveFocus();
        } else {
            event.accepted = false;
        }
    }

    QQC2.Action {
        id: addBluetoothDeviceAction

        text: root.addDeviceAction.text
        icon.name: root.addDeviceAction.icon.name

        onTriggered: source => root.addDeviceAction.trigger()
    }

    // Unlike the associated Plasma Action, this one is for a non-checkable button
    QQC2.Action {
        id: toggleBluetoothAction

        text: i18n("Enable")
        icon.name: root.toggleBluetoothAction.icon.name

        onTriggered: source => root.toggleBluetoothAction.trigger()
    }

    header: Toolbar {
        id: toolbar

        plasmoidItem: root.plasmoidItem
        addDeviceAction: root.addDeviceAction
        toggleBluetoothAction: root.toggleBluetoothAction
        toggleBadgeAction: root.toggleBadgeAction
        configureAction: root.configureAction

        visible: BluezQt.Manager.adapters.length > 0
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

            model: BluezQt.Manager.bluetoothOperational ? devicesModel : null
            clip: true
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds

            spacing: Kirigami.Units.smallSpacing

            // No topMargin because ListSectionHeader brings its own
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
            bottomMargin: Kirigami.Units.largeSpacing

            section.property: "Section"
            section.delegate: PlasmaExtras.ListSectionHeader {
                required property string section
                width: listView.width - listView.leftMargin - listView.rightMargin
                text: section
            }
            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: Kirigami.Units.shortDuration
            highlightResizeDuration: Kirigami.Units.shortDuration
            delegate: DeviceItem {}

            Keys.onUpPressed: event => {
                if (listView.currentIndex === 0) {
                    listView.currentIndex = -1;
                    toolbar.onSwitch.forceActiveFocus(Qt.BacktabFocusReason);
                } else {
                    event.accepted = false;
                }
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (4 * Kirigami.Units.gridUnit)
                active: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || !BluezQt.Manager.bluetoothOperational || root.emptyList
                sourceComponent: PlasmaExtras.PlaceholderMessage {
                    iconName: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown || !BluezQt.Manager.bluetoothOperational ? "network-bluetooth" : "network-bluetooth-activated"

                    text: {
                        // We cannot use the adapter count here because that can be zero when
                        // bluetooth is disabled even when there are physical devices
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return i18n("No Bluetooth adapters available");
                        } else if (!BluezQt.Manager.bluetoothOperational) {
                            return i18n("Bluetooth is disabled");
                        } else if (root.emptyList) {
                            return i18n("No devices paired");
                        } else {
                            return "";
                        }
                    }

                    helpfulAction: {
                        if (BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown) {
                            return null;
                        } else if (!BluezQt.Manager.bluetoothOperational) {
                            return toggleBluetoothAction;
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
