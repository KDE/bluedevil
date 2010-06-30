include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_BUILD_DIR}
   ${KDE4_INCLUDES}
)

set(audio_SRCS audio.cpp)

qt4_add_dbus_interface(audio_SRCS
    org.bluez.Audio.xml
    audio_interface)

kde4_add_plugin(audio ${audio_SRCS})

target_link_libraries(audio ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} bluedevilaction bluedevil)

install(TARGETS audio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES audio.desktop DESTINATION ${SERVICES_INSTALL_DIR})

