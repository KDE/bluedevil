set(audio_SRCS audio.cpp)

kde4_add_plugin(audio ${audio_SRCS})

target_link_libraries(audio ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} bluedevilservice)

install(TARGETS audio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES audio.desktop DESTINATION ${SERVICES_INSTALL_DIR})

