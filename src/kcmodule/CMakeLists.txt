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

add_library(kcm_bluedevildevices MODULE ${kcm_bluedevildevices_PART_SRCS})
add_library(kcm_bluedeviladapters MODULE ${kcm_bluedeviladapters_PART_SRCS})
add_library(kcm_bluedevilglobal MODULE ${kcm_bluedevilglobal_PART_SRCS} ${kcm_bluedevilglobal_PART_SRCS_UI})

kcoreaddons_desktop_to_json(kcm_bluedevildevices bluedevildevices.desktop)
kcoreaddons_desktop_to_json(kcm_bluedeviladapters bluedeviladapters.desktop)
kcoreaddons_desktop_to_json(kcm_bluedevilglobal bluedevilglobal.desktop)

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

install(TARGETS kcm_bluedevildevices
                kcm_bluedeviladapters
                kcm_bluedevilglobal DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES bluedevildevices.desktop
              bluedeviladapters.desktop
              bluedevilglobal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
