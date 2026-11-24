# SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS QuickTest)

add_test(
    NAME batteryutils_test
    COMMAND $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qmltestrunner> -import ${CMAKE_BINARY_DIR}/bin -input ${CMAKE_CURRENT_SOURCE_DIR}/tst_batteryutils.qml
)
set_tests_properties(batteryutils_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LANGUAGE=en_US")

# kcmshell6 < 6.27.0 doesn't know the --smoke-test param, so skip this test for
# now.
# TODO: Drop this when KF6_MIN_VERSION >= 6.27.0
if(KF6KCMUtils_VERSION VERSION_GREATER_EQUAL "6.27.0")
    add_test(NAME kcm_smoketest COMMAND kcmshell6 --smoke-test kcm_bluetooth)
    set_tests_properties(kcm_smoketest PROPERTIES
        ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
    )
endif()
