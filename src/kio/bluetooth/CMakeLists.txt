set(kio_bluetooth_PART_SRCS
   kiobluetooth.cpp)

set(kded_bluedevil.xml ${CMAKE_CURRENT_SOURCE_DIR}/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE "types.h")
qt5_add_dbus_interface(kio_bluetooth_PART_SRCS ${kded_bluedevil.xml} kdedbluedevil)

add_library(kio_bluetooth MODULE ${kio_bluetooth_PART_SRCS})

target_link_libraries(kio_bluetooth
    Qt5::Core
    Qt5::DBus
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    ${LibBlueDevil_LIBRARIES}
)

########### install files ###############
install(TARGETS kio_bluetooth DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluetooth.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES bluetooth-network.desktop DESTINATION  ${DATA_INSTALL_DIR}/remoteview)
