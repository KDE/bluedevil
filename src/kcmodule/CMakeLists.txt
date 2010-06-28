set(kcm_bluedevildevices_PART_SRCS bluedevildevices.cpp kded.cpp)
set(kcm_bluedeviladapters_PART_SRCS bluedeviladapters.cpp kded.cpp)

kde4_add_plugin(kcm_bluedevildevices ${kcm_bluedevildevices_PART_SRCS})
kde4_add_plugin(kcm_bluedeviladapters ${kcm_bluedeviladapters_PART_SRCS})

target_link_libraries(kcm_bluedevildevices ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)
target_link_libraries(kcm_bluedeviladapters ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)

install(TARGETS kcm_bluedevildevices
                kcm_bluedeviladapters DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevildevices.desktop
              bluedeviladapters.desktop DESTINATION ${SERVICES_INSTALL_DIR})
