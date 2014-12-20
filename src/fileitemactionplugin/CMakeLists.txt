set(fileitemactionplugin_SRCS
   sendfileitemaction.cpp)

set(kded_bluedevil.xml ${CMAKE_CURRENT_SOURCE_DIR}/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE "types.h")
qt4_add_dbus_interface(fileitemactionplugin_SRCS ${kded_bluedevil.xml} kded_bluedevil)

kde4_add_plugin(bluetoothfiletiemaction ${fileitemactionplugin_SRCS})
target_link_libraries(bluetoothfiletiemaction ${KDE4_KIO_LIBS})
install(TARGETS bluetoothfiletiemaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevilsendfile.desktop DESTINATION ${SERVICES_INSTALL_DIR})
