set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
)

QT4_ADD_DBUS_INTERFACE(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/manager.xml obexftpmanager)
QT4_ADD_DBUS_INTERFACE(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/session.xml obexftpsession)

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

