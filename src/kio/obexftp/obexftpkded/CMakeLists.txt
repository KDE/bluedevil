set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
    obexsession.cpp
    obexftp/listdirjob.cpp
)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/actionplugins/sendfile/helper/)
qt4_add_dbus_interface(kded_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Client1.xml obexd_client)
qt4_add_dbus_interface(kded_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.ObjectPush1.xml obexd_push)

set(FILE_TRANSFER_FILE obexftp/org.bluez.obex.FileTransfer1.xml)
set_source_files_properties(${FILE_TRANSFER_FILE} PROPERTIES INCLUDE "ObexFtpDaemon.h")
qt4_add_dbus_interface(kded_obexftp_SRCS ${FILE_TRANSFER_FILE} obexd_file_transfer)

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

