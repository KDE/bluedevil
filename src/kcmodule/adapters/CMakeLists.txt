set(kcm_bluedeviladapters_PART_SRCS
    adapters.cpp
    ../common/systemcheck.cpp)

set(kded.xml ${KDED_DBUS_INTERFACE})
qt5_add_dbus_interface(kcm_bluedeviladapters_PART_SRCS ${kded.xml} kded)

kconfig_add_kcfg_files(kcm_bluedeviladapters_PART_SRCS ../../settings/globalsettings.kcfgc)

add_library(kcm_bluedeviladapters MODULE ${kcm_bluedeviladapters_PART_SRCS})
kcoreaddons_desktop_to_json(kcm_bluedeviladapters bluedeviladapters.desktop)

target_link_libraries(kcm_bluedeviladapters
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::ConfigWidgets
    KF5::BluezQt)

install(TARGETS kcm_bluedeviladapters DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES bluedeviladapters.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
