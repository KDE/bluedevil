set(kcm_bluedevildevices_PART_SRCS
    bluedevildevices.cpp
    systemcheck.cpp
    devicedetails.cpp)

set(kcm_bluedeviladapters_PART_SRCS
    bluedeviladapters.cpp
    systemcheck.cpp)

set(kcm_bluedevilglobal_PART_SRCS
    bluedevilglobal.cpp
    systemcheck.cpp
)

set(kded.xml ${KDED_DBUS_INTERFACE})
qt5_add_dbus_interface(kcm_bluedevildevices_PART_SRCS ${kded.xml} kded)
qt5_add_dbus_interface(kcm_bluedeviladapters_PART_SRCS ${kded.xml} kded)
qt5_add_dbus_interface(kcm_bluedevilglobal_PART_SRCS ${kded.xml} kded)

ki18n_wrap_ui(kcm_bluedevilglobal_PART_SRCS_UI bluedevilglobal.ui)

kconfig_add_kcfg_files(kcm_bluedevilglobal_PART_SRCS
                    ../settings/filereceiversettings.kcfgc
                    ../settings/globalsettings.kcfgc)

kconfig_add_kcfg_files(kcm_bluedevildevices_PART_SRCS ../settings/globalsettings.kcfgc)

kconfig_add_kcfg_files(kcm_bluedeviladapters_PART_SRCS ../settings/globalsettings.kcfgc)

kcoreaddons_add_plugin(kcm_bluedevildevices INSTALL_NAMESPACE "." JSON bluedevildevices.json SOURCES ${kcm_bluedevildevices_PART_SRCS})
kcoreaddons_add_plugin(kcm_bluedeviladapters INSTALL_NAMESPACE "." JSON bluedeviladapters.json SOURCES ${kcm_bluedeviladapters_PART_SRCS})
kcoreaddons_add_plugin(kcm_bluedevilglobal INSTALL_NAMESPACE "." JSON bluedevilglobal.json SOURCES ${kcm_bluedevilglobal_PART_SRCS} ${kcm_bluedevilglobal_PART_SRCS_UI})

target_link_libraries(kcm_bluedevildevices
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::IconThemes
    KF5::ConfigWidgets
    KF5::BluezQt)

target_link_libraries(kcm_bluedeviladapters
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::ConfigWidgets
    KF5::BluezQt)

target_link_libraries(kcm_bluedevilglobal
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::KIOWidgets
    KF5::ConfigWidgets
    KF5::BluezQt)
