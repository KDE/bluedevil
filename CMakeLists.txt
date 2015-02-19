project(bluedevil)
set(PROJECT_VERSION "5.2.1")

cmake_minimum_required(VERSION 2.8.12)

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(QT_MIN_VERSION "5.2.0")
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
    KIO)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(LibBlueDevil 5.0 REQUIRED)
find_package(SharedMimeInfo REQUIRED)

include_directories(${LibBlueDevil_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
