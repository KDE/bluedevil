find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(LibBlueDevil REQUIRED)
find_package(SharedMimeInfo REQUIRED)

include_directories(${KDE4_INCLUDES} ${LibBlueDevil_INCLUDE_DIR})

add_subdirectory(src)
