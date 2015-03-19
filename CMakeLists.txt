project(bluedevil)
set(PROJECT_VERSION "5.2.90")
set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12)

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(QT_MIN_VERSION "5.4.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    DBus)

find_package(KF5 MODULE REQUIRED COMPONENTS
    CoreAddons
    WidgetsAddons
    DBusAddons
    Notifications
    IconThemes
    I18n
    KIO
    BluezQt)

find_package(KDED REQUIRED)
find_package(SharedMimeInfo REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
