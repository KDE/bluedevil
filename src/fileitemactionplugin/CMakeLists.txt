set(fileitemactionplugin_SRCS
   sendfileitemaction.cpp)

set(kded_bluedevil.xml ${CMAKE_CURRENT_SOURCE_DIR}/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE "types.h")
qt5_add_dbus_interface(fileitemactionplugin_SRCS ${kded_bluedevil.xml} kded_bluedevil)

add_library(bluetoothfileitemaction MODULE ${fileitemactionplugin_SRCS})
kcoreaddons_desktop_to_json(bluetoothfileitemaction bluedevilsendfile.desktop)
target_link_libraries(bluetoothfileitemaction
    Qt5::Widgets
    KF5::I18n
    KF5::KIOFileWidgets
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluetoothfileitemaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevilsendfile.desktop DESTINATION ${SERVICES_INSTALL_DIR})
