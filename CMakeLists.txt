project(bluedevil)

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_BLUEDEVIL_VERSION_MAJOR 2)
set(CMAKE_BLUEDEVIL_VERSION_MINOR 0)
set(CMAKE_BLUEDEVIL_VERSION_PATCH 0)
set(CMAKE_BLUEDEVIL_VERSION_STRING  "${CMAKE_BLUEDEVIL_VERSION_MAJOR}.${CMAKE_BLUEDEVIL_VERSION_MINOR}.${CMAKE_BLUEDEVIL_VERSION_PATCH}")

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(QT_MIN_VERSION "5.2.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    DBus)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    CoreAddons
    WidgetsAddons
    DBusAddons
    Notifications
    IconThemes
    I18n
    KIO
    QBluez)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(SharedMimeInfo REQUIRED)

add_subdirectory(src)
