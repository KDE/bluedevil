set(input_SRCS input.cpp)

kde4_add_plugin(input ${input_SRCS})

target_link_libraries(input ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} bluedevilservice)

install(TARGETS input DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES input.desktop DESTINATION ${SERVICES_INSTALL_DIR})

