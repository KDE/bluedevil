
set(kded_bluedevil_SRCS
    bluedevildaemon.cpp bluedevildaemon.h
    devicemonitor.cpp devicemonitor.h
    bluezagent.cpp bluezagent.h
    obexftp.cpp
    obexagent.cpp obexagent.h
    receivefilejob.cpp receivefilejob.h
    helpers/requestauthorization.cpp helpers/requestauthorization.h
    helpers/requestconfirmation.cpp helpers/requestconfirmation.h
    helpers/requestpin.cpp helpers/requestpin.h
)

ecm_qt_declare_logging_category(kded_bluedevil_SRCS
    HEADER bluedevil_kded.h
    IDENTIFIER BLUEDEVIL_KDED_LOG
    CATEGORY_NAME org.kde.plasma.bluedevil.kded
    DESCRIPTION "BlueDevil (kded)"
    EXPORT BLUEDEVIL
)

ki18n_wrap_ui(kded_bluedevil_SRCS
    helpers/requestpin.ui
)

kconfig_add_kcfg_files(kded_bluedevil_SRCS GENERATE_MOC ../settings/filereceiversettings.kcfgc)
kconfig_add_kcfg_files(kded_bluedevil_SRCS GENERATE_MOC ../settings/bluedevilglobalsettings.kcfgc)

kcoreaddons_add_plugin(kded_bluedevil INSTALL_NAMESPACE "kf6/kded" SOURCES ${kded_bluedevil_SRCS})
set_target_properties(kded_bluedevil PROPERTIES OUTPUT_NAME bluedevil)

target_link_libraries(kded_bluedevil
    Qt::Gui

    KF6::BluezQt
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::Notifications
    KF6::WindowSystem
)
