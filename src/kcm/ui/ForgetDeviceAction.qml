/**
 * SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.bluezqt as BluezQt
import org.kde.bluedevil.components as BluedevilComponents

QQC2.Action {
    required property BluedevilComponents.ForgetDeviceDialog dialog
    required property BluezQt.Device device

    text: i18nc("@action:button %1 is the name of a Bluetooth device", "Forget \"%1\"", device?.name ?? "");
    icon.name: "edit-delete-remove-symbolic"

    onTriggered: source => {
        // Device may get deleted, in which case the action becomes invalid and should not be triggered.
        if (dialog !== null && device !== null) {
            dialog.open(device);
        }
    }
}
