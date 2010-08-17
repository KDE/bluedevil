add_subdirectory(helper)
set(bluedevilaudioactionplugin_SRCS audio.cpp)

qt4_add_dbus_interface(bluedevilaudioactionplugin_SRCS
    org.bluez.Audio.xml
    audio_interface)

kde4_add_plugin(bluedevilaudioactionplugin ${bluedevilaudioactionplugin_SRCS})

target_link_libraries(bluedevilaudioactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS bluedevilaudioactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-audio.desktop DESTINATION ${SERVICES_INSTALL_DIR})
