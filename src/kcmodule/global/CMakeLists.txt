set(kcm_bluedevilglobal_PART_SRCS
    global.cpp
    ../common/systemcheck.cpp
)

set(kded.xml ${KDED_DBUS_INTERFACE})
qt5_add_dbus_interface(kcm_bluedevilglobal_PART_SRCS ${kded.xml} kded)

ki18n_wrap_ui(kcm_bluedevilglobal_PART_SRCS_UI global.ui)

kconfig_add_kcfg_files(kcm_bluedevilglobal_PART_SRCS
                    ../../settings/filereceiversettings.kcfgc
                    ../../settings/globalsettings.kcfgc)

add_library(kcm_bluedevilglobal MODULE ${kcm_bluedevilglobal_PART_SRCS} ${kcm_bluedevilglobal_PART_SRCS_UI})
kcoreaddons_desktop_to_json(kcm_bluedevilglobal bluedevilglobal.desktop)

target_link_libraries(kcm_bluedevilglobal
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::KIOWidgets
    KF5::ConfigWidgets
    KF5::BluezQt)

install(TARGETS kcm_bluedevilglobal DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevilglobal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
