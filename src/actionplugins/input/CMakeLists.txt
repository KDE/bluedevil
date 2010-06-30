include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_BUILD_DIR}
   ${KDE4_INCLUDES}
)

set(input_SRCS input.cpp)

qt4_add_dbus_interface(input_SRCS
    org.bluez.Input.xml
    input_interface)

kde4_add_plugin(input ${input_SRCS})

target_link_libraries(input ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} bluedevilaction bluedevil)

install(TARGETS input DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES input.desktop DESTINATION ${SERVICES_INSTALL_DIR})

