set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    devicemonitor.cpp
    bluezagent.cpp
    debug_p.cpp
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
set_target_properties(kded_bluedevil PROPERTIES OUTPUT_NAME bluedevil)

kcoreaddons_desktop_to_json(kded_bluedevil bluedevil.desktop)

target_link_libraries(kded_bluedevil
    Qt5::Gui
    KF5::I18n
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::Notifications
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::IconThemes
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_bluedevil DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
