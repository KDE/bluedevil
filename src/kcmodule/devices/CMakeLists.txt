set(kcm_bluedevildevices_PART_SRCS
    devices.cpp
    devicedetails.cpp
    ../common/systemcheck.cpp)

set(kded.xml ${KDED_DBUS_INTERFACE})
qt5_add_dbus_interface(kcm_bluedevildevices_PART_SRCS ${kded.xml} kded)

ki18n_wrap_ui(kcm_bluedevildevices_PART_SRCS_UI devices.ui devicedetails.ui)

kconfig_add_kcfg_files(kcm_bluedevildevices_PART_SRCS ../../settings/globalsettings.kcfgc)

add_library(kcm_bluedevildevices MODULE ${kcm_bluedevildevices_PART_SRCS} ${kcm_bluedevildevices_PART_SRCS_UI})
kcoreaddons_desktop_to_json(kcm_bluedevildevices bluedevildevices.desktop)

target_link_libraries(kcm_bluedevildevices
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::IconThemes
    KF5::ConfigWidgets
    KF5::BluezQt)

install(TARGETS kcm_bluedevildevices DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevildevices.desktop DESTINATION ${SERVICES_INSTALL_DIR})
