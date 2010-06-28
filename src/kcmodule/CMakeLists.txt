set(kcm_bluedevil_PART_SRCS bluedevildevices.cpp kded.cpp)

kde4_add_plugin(kcm_bluedevildevices ${kcm_bluedevil_PART_SRCS})

target_link_libraries(kcm_bluedevildevices ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)

install(TARGETS kcm_bluedevildevices  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevildevices.desktop DESTINATION ${SERVICES_INSTALL_DIR})
