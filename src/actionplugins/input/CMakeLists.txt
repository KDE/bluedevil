add_subdirectory(helper)

set(bluedevilinputactionplugin_SRCS input.cpp)

qt4_add_dbus_interface(bluedevilinputactionplugin_SRCS
    org.bluez.Input.xml
    input_interface)

kde4_add_plugin(bluedevilinputactionplugin ${bluedevilinputactionplugin_SRCS})

target_link_libraries(bluedevilinputactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS bluedevilinputactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-input.desktop DESTINATION ${SERVICES_INSTALL_DIR})
