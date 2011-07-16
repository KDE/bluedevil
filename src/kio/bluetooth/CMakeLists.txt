set(kio_bluetooth_PART_SRCS
   kiobluetooth.cpp)

set(kded_bluedevil.xml ${CMAKE_CURRENT_SOURCE_DIR}/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE "types.h")
qt4_add_dbus_interface(kio_bluetooth_PART_SRCS ${kded_bluedevil.xml} kdedbluedevil)

kde4_add_plugin(kio_bluetooth ${kio_bluetooth_PART_SRCS})

target_link_libraries(kio_bluetooth
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${LibBlueDevil_LIBRARIES}
)

########### install files ###############
install(TARGETS kio_bluetooth DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluetooth.protocol DESTINATION ${SERVICES_INSTALL_DIR})
