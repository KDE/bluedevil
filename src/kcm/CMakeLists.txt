add_library(kcm_bluetooth MODULE bluetooth.cpp bluetooth.h)

kconfig_add_kcfg_files(kcm_bluetooth GENERATE_MOC
                    ../settings/filereceiversettings.kcfgc)

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

install(FILES kcm_bluetooth.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(TARGETS kcm_bluetooth DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
kpackage_install_package(package kcm_bluetooth kcms)
