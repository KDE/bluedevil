set(kded_bluedevil_SRCS
    bluedevildaemon.cpp
    devicemonitor.cpp
    bluezagent.cpp
    debug_p.cpp
    obexftp.cpp
    obexagent.cpp
    receivefilejob.cpp
    helpers/requestauthorization.cpp
    helpers/requestconfirmation.cpp
    helpers/requestpin.cpp
)

ki18n_wrap_ui(kded_bluedevil_SRCS
    helpers/requestpin.ui)

kconfig_add_kcfg_files(kded_bluedevil_SRCS GENERATE_MOC ../settings/filereceiversettings.kcfgc)

kcoreaddons_add_plugin(kded_bluedevil INSTALL_NAMESPACE "kf5/kded" JSON bluedevil.json SOURCES ${kded_bluedevil_SRCS})
set_target_properties(kded_bluedevil PROPERTIES OUTPUT_NAME bluedevil)

target_link_libraries(kded_bluedevil
    Qt5::Gui
    KF5::I18n
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::Notifications
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::WindowSystem
    KF5::BluezQt
)
