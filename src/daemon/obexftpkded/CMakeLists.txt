set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
)

qt4_add_dbus_interface(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/org.openobex.xml obexdmanager)
qt4_add_dbus_interface(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/org.openobex.client.xml obexdclient)

kde4_add_plugin(kded_obexftpdaemon
    ${kded_obexftp_SRCS}
)

target_link_libraries(kded_obexftpdaemon
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_obexftpdaemon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES obexftpdaemon.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

