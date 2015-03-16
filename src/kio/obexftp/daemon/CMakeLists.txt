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
set_target_properties(kded_obexftpdaemon PROPERTIES OUTPUT_NAME obexftpdaemon)

kcoreaddons_desktop_to_json(kded_obexftpdaemon obexftpdaemon.desktop)

target_link_libraries(kded_obexftpdaemon
    Qt5::Core
    KF5::I18n
    KF5::DBusAddons
    KF5::KIOCore
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_obexftpdaemon DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
