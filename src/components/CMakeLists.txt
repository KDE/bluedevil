# SPDX-FileCopyrightText: 2026 Andrew Gigena <mail@andrewgigena.dev>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

set(COMPONENTS_QML_FILES
    ForgetDeviceDialog.qml
    qmldir
)

install(FILES ${COMPONENTS_QML_FILES}
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/bluedevil/components
)
