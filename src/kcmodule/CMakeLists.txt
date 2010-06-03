set(kcm_bluedevil_PART_SRCS bluedevil.cpp kded.cpp)

kde4_add_plugin(kcm_bluetooth ${kcm_bluedevil_PART_SRCS})

target_link_libraries(kcm_bluetooth  ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kcm_bluetooth  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil.desktop DESTINATION ${SERVICES_INSTALL_DIR})
