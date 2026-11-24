/*
    SPDX-FileCopyrightText: 2026 Matthias Kurz <m.kurz@irregular.at>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma Singleton

import QtQml
import org.kde.ki18n

QtObject {
    id: root

    readonly property KI18nContext __i18n: KI18nContext {
        translationDomain: "bluedevil"
    }

    function componentOrder(identifier) {
        switch (identifier.toLowerCase()) {
        case "left":
            return 0;
        case "right":
            return 1;
        case "case":
            return 2;
        default:
            return 3;
        }
    }

    function displayBatteries(device) {
        const components = [];
        // Older BluezQt versions only expose the single device.battery.
        const batteries = device.batteries ?? [];

        for (let i = 0; i < batteries.length; ++i) {
            const battery = batteries[i];
            if (battery.ubi !== undefined && battery.ubi !== device.ubi) {
                components.push(battery);
            }
        }

        if (components.length === 0) {
            return device.battery === null ? [] : [device.battery];
        }

        components.sort((left, right) => {
            const leftIdentifier = left.identifier ?? "";
            const rightIdentifier = right.identifier ?? "";
            const order = root.componentOrder(leftIdentifier) - root.componentOrder(rightIdentifier);
            return order !== 0 ? order : leftIdentifier.localeCompare(rightIdentifier);
        });
        return components;
    }

    function componentName(identifier) {
        switch (identifier.toLowerCase()) {
        case "left":
            return root.__i18n.i18nc("@label Bluetooth battery for the left earbud", "Left");
        case "right":
            return root.__i18n.i18nc("@label Bluetooth battery for the right earbud", "Right");
        case "case":
            return root.__i18n.i18nc("@label Bluetooth battery for the charging case", "Case");
        default:
            return identifier;
        }
    }

    function batteryLabel(battery) {
        const identifier = battery.identifier ?? "";
        // Missing validity means the legacy numeric API, not an unknown level.
        const percentageValid = battery.percentageValid ?? true;

        if (identifier === "") {
            return percentageValid
                ? root.__i18n.i18n("%1% Battery", battery.percentage)
                : root.__i18n.i18nc("@info Bluetooth battery level is unknown", "Battery level unknown");
        }

        const name = root.componentName(identifier);
        return percentageValid
            ? root.__i18n.i18nc("@info Bluetooth component battery level; %1 is the component name and %2 is the percentage", "%1: %2%", name, battery.percentage)
            : root.__i18n.i18nc("@info Bluetooth component battery level; %1 is the component name", "%1: Unknown", name);
    }

    function batteryLabels(device) {
        return root.displayBatteries(device).map(battery => root.batteryLabel(battery));
    }
}
