kcoreaddons_add_plugin(kcm_bluetooth
    INSTALL_NAMESPACE "plasma/kcms/systemsettings"
    SOURCES
        bluetooth.cpp
        bluetooth.h
)

kcmutils_generate_desktop_file(kcm_bluetooth)
kconfig_add_kcfg_files(kcm_bluetooth GENERATE_MOC
    ../settings/filereceiversettings.kcfgc
)

target_link_libraries(kcm_bluetooth
    Qt::Gui
    Qt::Qml
    Qt::DBus
    KF5::CoreAddons
    KF5::ConfigGui
    KF5::QuickAddons
    KF5::I18n
    KF5::BluezQt
    KF5::KIOGui
)

kpackage_install_package(package kcm_bluetooth kcms)
