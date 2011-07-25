add_subdirectory(helper)
set(bluedevilnetworkactionplugin_SRCS network.cpp)

qt4_add_dbus_interface(bluedevilnetworkactionplugin_SRCS org.bluez.Serial.xml serial_interface)

kde4_add_plugin(bluedevilnetworkactionplugin ${bluedevilnetworkactionplugin_SRCS})

target_link_libraries(bluedevilnetworkactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS bluedevilnetworkactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-network.desktop DESTINATION ${SERVICES_INSTALL_DIR})
