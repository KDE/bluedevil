find_package(KDE4 REQUIRED)

set(CMAKE_BLUEDEVIL_VERSION_MAJOR 2)
set(CMAKE_BLUEDEVIL_VERSION_MINOR 0)
set(CMAKE_BLUEDEVIL_VERSION_PATCH 0)
set(CMAKE_BLUEDEVIL_VERSION_STRING  "${CMAKE_BLUEDEVIL_VERSION_MAJOR}.${CMAKE_BLUEDEVIL_VERSION_MINOR}.${CMAKE_BLUEDEVIL_VERSION_PATCH}")
configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(LibBlueDevil 2.0 REQUIRED)
find_package(SharedMimeInfo REQUIRED)

include_directories(${KDE4_INCLUDES} ${LibBlueDevil_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
