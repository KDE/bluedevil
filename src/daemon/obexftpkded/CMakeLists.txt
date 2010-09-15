set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
    obexsession.cpp
)

QT4_ADD_DBUS_INTERFACE(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/session.xml obexftpsession)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/obexftp/manager.xml PROPERTIES INCLUDE "qstringmap.h")
QT4_ADD_DBUS_INTERFACE(kded_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/manager.xml obexftpmanager)

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

