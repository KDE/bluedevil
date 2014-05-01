set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    bluezagent.cpp
    filereceiver/obexagent.cpp
    filereceiver/filereceiver.cpp
    filereceiver/receivefilejob.cpp
)

qt5_add_dbus_interface(kded_bluedevil_SRCS
    filereceiver/org.bluez.obex.AgentManager1.xml
    obex_agent_manager)

qt5_add_dbus_interface(kded_bluedevil_SRCS
    filereceiver/org.bluez.obex.Transfer1.xml
    obex_transfer)

qt5_add_dbus_interface(kded_bluedevil_SRCS
    filereceiver/org.bluez.obex.Session1.xml
    obex_session)

qt5_add_dbus_interface(kded_bluedevil_SRCS
    filereceiver/org.freedesktop.DBus.Properties.xml
    dbus_properties)

kconfig_add_kcfg_files(kded_bluedevil_SRCS ../../settings/filereceiversettings.kcfgc)

add_library(kded_bluedevil MODULE ${kded_bluedevil_SRCS})

kservice_desktop_to_json(kded_bluedevil bluedevil.desktop)

target_link_libraries(kded_bluedevil
    KF5::DBusAddons
    KF5::KDELibs4Support
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_bluedevil DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
