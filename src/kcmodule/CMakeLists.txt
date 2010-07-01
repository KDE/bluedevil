include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

set(kcm_bluedevildevices_PART_SRCS bluedevildevices.cpp systemcheck.cpp kded.cpp)
set(kcm_bluedeviladapters_PART_SRCS bluedeviladapters.cpp systemcheck.cpp kded.cpp)
set(kcm_bluedeviltransfer_PART_SRCS bluedeviltransfer.cpp systemcheck.cpp kded.cpp)

kde4_add_ui_files(kcm_bluedeviltransfer_PART_SRCS_UI transfer.ui)
kde4_add_kcfg_files(kcm_bluedeviltransfer_PART_SRCS ../daemon/helpers/filereceiver/settings.kcfgc)

kde4_add_plugin(kcm_bluedevildevices ${kcm_bluedevildevices_PART_SRCS})
kde4_add_plugin(kcm_bluedeviladapters ${kcm_bluedeviladapters_PART_SRCS})
kde4_add_plugin(kcm_bluedeviltransfer ${kcm_bluedeviltransfer_PART_SRCS} ${kcm_bluedeviltransfer_PART_SRCS_UI})

qt4_automoc(${kcm_bluedevildevices}
            ${kcm_bluedeviladapters}
            ${kcm_bluedeviltransfer})

target_link_libraries(kcm_bluedevildevices ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)
target_link_libraries(kcm_bluedeviladapters ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)
target_link_libraries(kcm_bluedeviltransfer ${KDE4_KIO_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QTGUI_LIBRARY} bluedevil)

install(TARGETS kcm_bluedevildevices
                kcm_bluedeviladapters
                kcm_bluedeviltransfer DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES bluedevildevices.desktop
              bluedeviladapters.desktop
              bluedeviltransfer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
