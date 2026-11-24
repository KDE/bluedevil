/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.bluezqt as BluezQt
import org.kde.bluedevil.kcm

KCMUtils.SimpleKCM {
    id: root

    title: i18nc("@title:window page title", "Settings")

    // FIXME: Manager.adapters property does not have any NOTIFY hook.
    // Somehow even the name won't update in the ComboBox
    readonly property BluezQt.Adapter adapter: BluezQt.Manager.adapters[box.currentIndex] ?? null

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                visible: box.count > 1
                title: i18nc("@label:listbox device selector in case of multiple bt adapter", "Device:")
                contentItem: QQC2.ComboBox {
                    id: box
                    model: BluezQt.Manager.adapters
                    textRole: "name"
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:textbox editable name of the selected device (adapter)", "Name:")
                contentItem: QQC2.TextField {
                    text: root.adapter.name
                    onEditingFinished: root.adapter.name = text
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@info bluetooth address of the selected device (adapter)", "Address:")
                contentItem: QQC2.Label {
                    text: root.adapter.address
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.Switch {
                    text: i18nc("@option:check switch", "Enable device")
                    checked: root.adapter.powered
                    onToggled: root.adapter.powered = checked
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.Switch {
                    text: i18nc("@option:check switch", "Make visible to other devices")
                    checked: root.adapter.discoverable
                    onToggled: root.adapter.discoverable = checked
                }
            }
        }

        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: loginStateRadioGroup }
            Kirigami.FormEntry {
                title: i18nc("@title:group completes e.g. … Enable Bluetooth", "On login:")
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio completes On login: …", "Enable Bluetooth")
                    QQC2.ButtonGroup.group: loginStateRadioGroup
                    checked: GlobalSettings.launchState === GlobalSettings.Enable
                    onToggled: {
                        if (enabled) {
                            GlobalSettings.launchState = GlobalSettings.Enable;
                            GlobalSettings.save()
                        }
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio completes On login: …", "Disable Bluetooth")
                    QQC2.ButtonGroup.group: loginStateRadioGroup
                    checked: GlobalSettings.launchState === GlobalSettings.Disable
                    onToggled: {
                        if (enabled) {
                            GlobalSettings.launchState = GlobalSettings.Disable
                            GlobalSettings.save()
                        }
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio completes On login: …", "Restore previous status")
                    QQC2.ButtonGroup.group: loginStateRadioGroup
                    checked: GlobalSettings.launchState === GlobalSettings.Remember
                    onToggled: {
                        if (enabled) {
                            GlobalSettings.launchState = GlobalSettings.Remember
                            GlobalSettings.save()
                        }
                    }
                }
            }
        }

        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: receivingFilesRadioGroup }
            Kirigami.FormEntry {
                title: i18nc("@title:group completes e.g. … Ask for confirmation", "When receiving files:")
                contentItem: QQC2.RadioButton {
                    checked: FileReceiverSettings.autoAccept === 0
                    text: i18nc("@option:radio completes When receiving files:…", "Ask for confirmation")
                    QQC2.ButtonGroup.group: receivingFilesRadioGroup
                    onClicked: {
                        FileReceiverSettings.autoAccept = 0
                        FileReceiverSettings.save()
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio completes When receiving files:…", "Accept for trusted devices")
                    checked: FileReceiverSettings.autoAccept === 1
                    QQC2.ButtonGroup.group: receivingFilesRadioGroup
                    onClicked: {
                        FileReceiverSettings.autoAccept = 1
                        FileReceiverSettings.save()
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio completes When receiving files:…", "Always accept")
                    QQC2.ButtonGroup.group: receivingFilesRadioGroup
                    checked: FileReceiverSettings.autoAccept === 2
                    onClicked: {
                        FileReceiverSettings.autoAccept = 2
                        FileReceiverSettings.save()
                    }
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:textbox", "Save files in:")
                contentItem: QQC2.TextField {
                    Layout.fillWidth: true
                    text: kcm.receiveFolderPath

                    onEditingFinished: {
                        kcm.receiveFolderPath = text
                        FileReceiverSettings.save()
                    }
                }
                trailingItems: [
                    QQC2.Button {
                        icon.name: "document-open-folder-symbolic"
                        display: QQC2.AbstractButton.IconOnly
                        text: i18nc("@action:button tooltip/accessible only", "Select folder")
                        onClicked: folderDialogLoader.active = true
                        QQC2.ToolTip.visible: hovered || visualFocus
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                ]
                Loader {
                    id: folderDialogLoader

                    active: false

                    sourceComponent: FolderDialog {
                        id: startupFileDialog
                        title: i18nc("@title:window dialog title", "Select folder")

                        currentFolder: FileReceiverSettings.saveUrl

                        onAccepted: {
                            kcm.receiveFolderPath = selectedFolder
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
}
