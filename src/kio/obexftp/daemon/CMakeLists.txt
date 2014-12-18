set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
    createsessionjob.cpp
)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/sendfile/)
qt4_add_dbus_interface(kded_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Client1.xml obexd_client)

set(OBJECT_MANAGER_FILE org.freedesktop.DBus.ObjectManager.xml)
set_source_files_properties(${OBJECT_MANAGER_FILE} PROPERTIES INCLUDE "../obexdtypes.h")
qt4_add_dbus_interface(kded_obexftp_SRCS org.freedesktop.DBus.ObjectManager.xml dbus_object_manager)

kde4_add_plugin(kded_obexftpdaemon
    ${kded_obexftp_SRCS}
)

target_link_libraries(kded_obexftpdaemon
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
)

install(TARGETS kded_obexftpdaemon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES obexftpdaemon.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

