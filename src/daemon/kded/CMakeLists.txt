set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    bluezagent.cpp
    debug_p.cpp
    filereceiver/obexagent.cpp
    filereceiver/filereceiver.cpp
    filereceiver/receivefilejob.cpp
)

kconfig_add_kcfg_files(kded_bluedevil_SRCS ../../settings/filereceiversettings.kcfgc)

add_library(kded_bluedevil MODULE ${kded_bluedevil_SRCS})

kservice_desktop_to_json(kded_bluedevil bluedevil.desktop)

target_link_libraries(kded_bluedevil
    Qt5::Gui
    KF5::I18n
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::Notifications
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::IconThemes
    KF5::QBluez
)

install(TARGETS kded_bluedevil DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
