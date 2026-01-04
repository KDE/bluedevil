# SPDX-FileCopyrightText: 2026 Andrew Gigena <mail@andrewgigena.dev>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

ecm_add_qml_module(bluedevilcomponents URI org.kde.bluedevil.components GENERATE_PLUGIN_SOURCE)

target_compile_definitions(bluedevilcomponents PRIVATE TRANSLATION_DOMAIN="bluedevil")

ecm_target_qml_sources(bluedevilcomponents SOURCES
     ForgetDeviceDialog.qml
 )

target_link_libraries(bluedevilcomponents PRIVATE
    Qt::Core
    Qt::Qml
    Qt::Quick
    KF6::BluezQt
    KF6::I18n
 )

ecm_finalize_qml_module(bluedevilcomponents)
