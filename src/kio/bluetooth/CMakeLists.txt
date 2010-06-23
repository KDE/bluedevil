include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${KDE4WORKSPACE_INCLUDE_DIR})
include(${QT_USE_FILE})

set(kio_bluetooth_PART_SRCS
   kiobluetooth.cpp)

kde4_add_plugin(kio_bluetooth ${kio_bluetooth_PART_SRCS})

target_link_libraries(kio_bluetooth
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_SOLID_LIBS}
  ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY}
  ${QT_QTTEST_LIBRARY}
)

########### install files ###############
install(TARGETS kio_bluetooth DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluetooth.protocol DESTINATION ${SERVICES_INSTALL_DIR})
