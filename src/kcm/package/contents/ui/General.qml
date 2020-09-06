/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Dialogs 1.3

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.bluedevil.kcm 1.0

SimpleKCM {

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
            id: radioGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("When receiving files:")
            checked: FileReceiverSettings.autoAccept == 0
            text: i18n("Ask for confirmation")
            QQC2.ButtonGroup.group: radioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 0
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Accept for trusted devices")
            checked: FileReceiverSettings.autoAccept == 1
            QQC2.ButtonGroup.group: radioGroup
            onClicked: {
                FileReceiverSettings.autoAccept = 1
                FileReceiverSettings.save()
            }
        }

        QQC2.RadioButton {
            text: i18n("Always accept")
            QQC2.ButtonGroup.group: radioGroup
            checked: FileReceiverSettings.autoAccept == 2
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
                icon.name: "folder"
                display: QQC2.AbstractButton.IconOnly
                text: i18n("Select folder")
                onClicked: folderDialogLoader.active = true
            }

            Loader {
                id: folderDialogLoader

                active: false

                sourceComponent: FileDialog {
                    id: startupFileDialog
                    title: i18n("Select folder")
                    selectFolder: true

                    folder: FileReceiverSettings.saveUrl

                    onAccepted: {
                        FileReceiverSettings.saveUrl = folder
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

