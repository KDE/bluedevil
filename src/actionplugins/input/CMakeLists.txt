add_subdirectory(helper)

set(inputactionplugin_SRCS input.cpp)

qt4_add_dbus_interface(inputactionplugin_SRCS
    org.bluez.Input.xml
    input_interface)

kde4_add_plugin(inputactionplugin ${inputactionplugin_SRCS})

target_link_libraries(inputactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS inputactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-input.desktop DESTINATION ${SERVICES_INSTALL_DIR})

