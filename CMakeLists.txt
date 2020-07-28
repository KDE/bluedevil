cmake_minimum_required(VERSION 3.0)

project(bluedevil)
set(PROJECT_VERSION "5.19.4")
set(PROJECT_VERSION_MAJOR 5)

set(QT_MIN_VERSION "5.14.0")
set(KF5_MIN_VERSION "5.70.0")

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Qml
    DBus)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    WidgetsAddons
    DBusAddons
    Notifications
    WindowSystem
    IconThemes
    Plasma
    I18n
    KIO
    BluezQt)

find_package(KDED ${KF5_MIN_VERSION} REQUIRED)
find_package(SharedMimeInfo REQUIRED)

find_package(KF5Kirigami2 ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5Kirigami2 PROPERTIES
    DESCRIPTION "A QtQuick based components set"
    TYPE RUNTIME
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)
include(KDEClangFormat)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
