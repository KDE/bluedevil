set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
    createsessionjob.cpp
)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/sendfile/)
qt5_add_dbus_interface(kded_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Client1.xml obexd_client)

set(OBJECT_MANAGER_FILE org.freedesktop.DBus.ObjectManager.xml)
set_source_files_properties(${OBJECT_MANAGER_FILE} PROPERTIES INCLUDE "../obexdtypes.h")
qt5_add_dbus_interface(kded_obexftp_SRCS org.freedesktop.DBus.ObjectManager.xml dbus_object_manager)

add_library(kded_obexftpdaemon MODULE ${kded_obexftp_SRCS})

kservice_desktop_to_json(kded_obexftpdaemon obexftpdaemon.desktop)

target_link_libraries(kded_obexftpdaemon
    KF5::DBusAddons
    KF5::KDELibs4Support
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_obexftpdaemon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES obexftpdaemon.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
