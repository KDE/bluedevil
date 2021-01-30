set(kio_bluetooth_SRCS
   kiobluetooth.cpp)

set(kded_bluedevil.xml ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil_types.h)
qt5_add_dbus_interface(kio_bluetooth_SRCS ${kded_bluedevil.xml} kdedbluedevil)

kconfig_add_kcfg_files(kio_bluetooth_SRCS GENERATE_MOC ../../settings/filereceiversettings.kcfgc)

add_library(kio_bluetooth MODULE ${kio_bluetooth_SRCS})

target_link_libraries(kio_bluetooth
    Qt::Core
    Qt::DBus
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    KF5::ConfigGui
    Qt::Network
)

target_include_directories(kio_bluetooth
    PUBLIC $<TARGET_PROPERTY:KF5::BluezQt,INTERFACE_INCLUDE_DIRECTORIES>)

########### install files ###############
install(TARGETS kio_bluetooth DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES bluetooth.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES bluetooth-network.desktop DESTINATION ${KDE_INSTALL_DATADIR}/remoteview)
