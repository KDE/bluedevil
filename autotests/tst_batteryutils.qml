/*
    SPDX-FileCopyrightText: 2026 Matthias Kurz <m.kurz@irregular.at>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtTest

import org.kde.bluedevil.components as BluedevilComponents

TestCase {
    name: "BatteryUtils"

    readonly property var aggregate: ({
        ubi: "/org/bluez/hci0/dev_00",
        identifier: "",
        percentage: 94,
        percentageValid: true
    })
    readonly property var leftBattery: ({
        ubi: "/org/bluez/hci0/dev_00/battery_left",
        identifier: "left",
        percentage: 95,
        percentageValid: true
    })
    readonly property var rightBattery: ({
        ubi: "/org/bluez/hci0/dev_00/battery_right",
        identifier: "right",
        percentage: 93,
        percentageValid: true
    })
    readonly property var batteryCase: ({
        ubi: "/org/bluez/hci0/dev_00/battery_case",
        identifier: "case",
        percentage: -1,
        percentageValid: false
    })

    // Match the older QML API: the battery exposes only its percentage,
    // and the device has no batteries property.
    Component {
        id: legacyDeviceComponent

        QtObject {
            id: legacyDevice

            readonly property string ubi: "/org/bluez/hci0/dev_00"
            property QtObject battery: QtObject {
                property int percentage: 94
            }
            readonly property var batteryLabels: BluedevilComponents.BatteryUtils.batteryLabels(legacyDevice)
        }
    }

    Component {
        id: componentDeviceComponent

        QtObject {
            id: componentDevice

            readonly property string ubi: "/org/bluez/hci0/dev_00"
            property QtObject aggregateBattery: QtObject {
                readonly property string ubi: componentDevice.ubi
                property string identifier: ""
                property int percentage: 94
                property bool percentageValid: true
            }
            property QtObject leftBattery: QtObject {
                readonly property string ubi: componentDevice.ubi + "/battery_left"
                property string identifier: "left"
                property int percentage: 95
                property bool percentageValid: true
            }
            property QtObject caseBattery: QtObject {
                readonly property string ubi: componentDevice.ubi + "/battery_case"
                property string identifier: "case"
                property int percentage: -1
                property bool percentageValid: false
            }
            property QtObject battery: aggregateBattery
            property list<QtObject> batteries: [aggregateBattery]
            readonly property var batteryLabels: BluedevilComponents.BatteryUtils.batteryLabels(componentDevice)
        }
    }

    function compareBatteries(actual, expected) {
        compare(actual.length, expected.length);
        for (let i = 0; i < expected.length; ++i) {
            compare(actual[i], expected[i]);
        }
    }

    function verifyBatteries(device, expected) {
        compareBatteries(BluedevilComponents.BatteryUtils.displayBatteries(device), expected);
    }

    function compareLabels(device, expected) {
        compare(BluedevilComponents.BatteryUtils.batteryLabels(device), expected);
    }

    function test_componentsReplaceAggregateAndUseLogicalOrder() {
        const device = {
            ubi: aggregate.ubi,
            battery: aggregate,
            batteries: [batteryCase, aggregate, rightBattery, leftBattery]
        };

        verifyBatteries(device, [leftBattery, rightBattery, batteryCase]);
        compareLabels(device, ["Left: 95%", "Right: 93%", "Case: Unknown"]);
    }

    function test_aggregateFallback() {
        const device = {
            ubi: aggregate.ubi,
            battery: aggregate,
            batteries: [aggregate]
        };

        verifyBatteries(device, [aggregate]);
        compareLabels(device, ["94% Battery"]);
    }

    function test_singleComponentWithoutAggregate() {
        const device = {
            ubi: aggregate.ubi,
            battery: leftBattery,
            batteries: [leftBattery]
        };

        verifyBatteries(device, [leftBattery]);
        compareLabels(device, ["Left: 95%"]);
    }

    function test_noBatteries() {
        const device = {
            ubi: aggregate.ubi,
            battery: null,
            batteries: []
        };

        verifyBatteries(device, []);
        compareLabels(device, []);
    }

    function test_legacyBattery_data() {
        return [
            { tag: "ordinary", percentage: 94 },
            { tag: "empty", percentage: 0 },
            { tag: "full", percentage: 100 }
        ];
    }

    function test_legacyBattery(data) {
        const battery = { percentage: data.percentage };
        const device = { ubi: aggregate.ubi, battery: battery };

        verifyBatteries(device, [battery]);
        compareLabels(device, ["%1% Battery".arg(data.percentage)]);
    }

    function test_legacyNoBattery() {
        const device = { ubi: aggregate.ubi, battery: null };

        verifyBatteries(device, []);
        compareLabels(device, []);
    }

    function test_legacyBatteryInList() {
        const battery = { percentage: 94 };
        const device = {
            ubi: aggregate.ubi,
            battery: battery,
            batteries: [battery]
        };

        verifyBatteries(device, [battery]);
        compareLabels(device, ["94% Battery"]);
    }

    function test_aggregatePercentageValidity_data() {
        return [
            { tag: "unknown", valid: false, label: "Battery level unknown" },
            { tag: "empty", valid: true, label: "0% Battery" }
        ];
    }

    function test_aggregatePercentageValidity(data) {
        const battery = {
            ubi: aggregate.ubi,
            identifier: "",
            percentage: 0,
            percentageValid: data.valid
        };
        const device = { ubi: aggregate.ubi, battery: battery, batteries: [battery] };

        verifyBatteries(device, [battery]);
        compareLabels(device, [data.label]);
    }

    function test_legacyQmlDeviceUpdates() {
        const device = createTemporaryObject(legacyDeviceComponent, this);
        verify(device !== null);
        const battery = device.battery;

        compare(device.batteries, undefined);
        compare(battery.ubi, undefined);
        compare(battery.identifier, undefined);
        compare(battery.percentageValid, undefined);
        compare(device.batteryLabels, ["94% Battery"]);

        battery.percentage = 0;
        compare(device.batteryLabels, ["0% Battery"]);

        device.battery = null;
        compare(device.batteryLabels, []);

        device.battery = battery;
        battery.percentage = 100;
        compare(device.batteryLabels, ["100% Battery"]);
    }

    function test_componentQmlDeviceUpdates() {
        const device = createTemporaryObject(componentDeviceComponent, this);
        verify(device !== null);
        function compareBoundLabels(expected) {
            compare(device.batteryLabels, expected);
        }

        compareBoundLabels(["94% Battery"]);
        device.batteries = [device.caseBattery, device.aggregateBattery, device.leftBattery];
        compareBoundLabels(["Left: 95%", "Case: Unknown"]);

        device.leftBattery.percentage = 0;
        compareBoundLabels(["Left: 0%", "Case: Unknown"]);
        device.leftBattery.percentageValid = false;
        // Unknown components still take precedence over a known aggregate.
        compareBoundLabels(["Left: Unknown", "Case: Unknown"]);
        device.leftBattery.percentageValid = true;
        compareBoundLabels(["Left: 0%", "Case: Unknown"]);

        device.caseBattery.percentage = 0;
        compareBoundLabels(["Left: 0%", "Case: Unknown"]);
        device.caseBattery.percentageValid = true;
        compareBoundLabels(["Left: 0%", "Case: 0%"]);

        device.leftBattery.identifier = "right";
        compareBoundLabels(["Right: 0%", "Case: 0%"]);
        device.batteries = [device.caseBattery, device.aggregateBattery];
        compareBoundLabels(["Case: 0%"]);
        device.batteries = [device.aggregateBattery];
        compareBoundLabels(["94% Battery"]);

        device.battery = null;
        device.batteries = [];
        compareBoundLabels([]);
    }
}
