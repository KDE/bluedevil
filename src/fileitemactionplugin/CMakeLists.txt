set(fileitemactionplugin_SRCS
   sendfileitemaction.cpp)

set(kded_bluedevil.xml ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil_types.h)
qt5_add_dbus_interface(fileitemactionplugin_SRCS ${kded_bluedevil.xml} kdedbluedevil)

add_library(bluetoothfileitemaction MODULE ${fileitemactionplugin_SRCS})
kcoreaddons_desktop_to_json(bluetoothfileitemaction bluedevilfileitemaction.desktop)

target_link_libraries(bluetoothfileitemaction
    Qt5::Widgets
    KF5::I18n
    KF5::KIOFileWidgets)
target_include_directories(bluetoothfileitemaction SYSTEM
    PUBLIC $<TARGET_PROPERTY:KF5::BluezQt,INTERFACE_INCLUDE_DIRECTORIES>)

install(TARGETS bluetoothfileitemaction DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES bluedevilfileitemaction.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
