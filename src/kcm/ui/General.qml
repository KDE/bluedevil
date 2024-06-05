/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.bluezqt as BluezQt
import org.kde.bluedevil.kcm

KCMUtils.SimpleKCM {

    title: i18n("Settings")

    Kirigami.FormLayout {
        id: form

        property QtObject adapter: BluezQt.Manager.adapters[box.currentIndex]

        QQC2.ComboBox {
            id: box
            Kirigami.FormData.label: i18n("Device:")
            model: BluezQt.Manager.adapters
            textRole: "name"
            visible: count > 1
        }

        QQC2.TextField {
            text: form.adapter.name
            Kirigami.FormData.label: i18n("Name:")
            onEditingFinished: form.adapter.name = text
        }

        QQC2.Label {
            text: form.adapter.address
            Kirigami.FormData.label: i18n("Address:")
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("Enabled:")
            checked: form.adapter.powered
            onToggled: form.adapter.powered = checked
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("Visible:")
            checked: form.adapter.discoverable
            onToggled: form.adapter.discoverable = checked
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QQC2.ButtonGroup {
            id: loginStateRadioGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("On login:")
            text: i18n("Enable Bluetooth")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: kcm.bluetoothStatusAtLogin === "enable"
            onToggled: {
                if (enabled) {
                    kcm.bluetoothStatusAtLogin = "enable";
                }
            }
        }
        QQC2.RadioButton {
            text: i18n("Disable Bluetooth")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: kcm.bluetoothStatusAtLogin === "disable"
            onToggled: {
                if (enabled) {
                    kcm.bluetoothStatusAtLogin = "disable"
                }
            }
        }
        QQC2.RadioButton {
            text: i18n("Restore previous status")
            QQC2.ButtonGroup.group: loginStateRadioGroup
            checked: kcm.bluetoothStatusAtLogin === "remember"
            onToggled: {
                if (enabled) {
                    kcm.bluetoothStatusAtLogin = "remember"
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QQC2.ButtonGroup {
            id: receivingFilesRadioGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("When receiving files:")
            checked: FileReceiverSettings.autoAccept === 0
            text: i18n("Ask for confirmation")
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 0
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Accept for trusted devices")
            checked: FileReceiverSettings.autoAccept === 1
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 1
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Always accept")
            QQC2.ButtonGroup.group: receivingFilesRadioGroup
            checked: FileReceiverSettings.autoAccept === 2
            onClicked: {
                FileReceiverSettings.autoAccept = 2
                FileReceiverSettings.save()
            }
        }

        Row {
            Kirigami.FormData.label: i18n("Save files in:")
            QQC2.TextField {
                function urlToPath(urlString) {
                    var s
                    if (urlString.startsWith("file:///")) {
                        var k = urlString.charAt(9) === ':' ? 8 : 7
                        s = urlString.substring(k)
                    } else {
                        s = urlString
                    }
                    return decodeURIComponent(s);
                }

                text: urlToPath(FileReceiverSettings.saveUrl.toString())

                onEditingFinished: {
                    FileReceiverSettings.saveUrl = Qt.resolvedUrl(text)
                    FileReceiverSettings.save()
                }
            }

            QQC2.Button {
                icon.name: "document-open-folder-symbolic"
                display: QQC2.AbstractButton.IconOnly
                text: i18n("Select folder")
                onClicked: folderDialogLoader.active = true
            }

            Loader {
                id: folderDialogLoader

                active: false

                sourceComponent: FolderDialog {
                    id: startupFileDialog
                    title: i18n("Select folder")

                    currentFolder: FileReceiverSettings.saveUrl

                    onAccepted: {
                        FileReceiverSettings.saveUrl = selectedFolder
                        folderDialogLoader.active = false
                        FileReceiverSettings.save()
                    }

                    onRejected: folderDialogLoader.active = false

                    Component.onCompleted: open()
                }
            }
        }
    }
}
