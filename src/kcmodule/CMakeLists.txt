set(kcm_bluedevildevices_PART_SRCS bluedevildevices.cpp systemcheck.cpp kded.cpp devicedetails.cpp)
set(kcm_bluedeviladapters_PART_SRCS bluedeviladapters.cpp systemcheck.cpp kded.cpp)
set(kcm_bluedeviltransfer_PART_SRCS
    bluedeviltransfer.cpp
    systemcheck.cpp
    kded.cpp
    columnresizer.cpp
    sharedfilesdialog/sharedfilesdialog.cpp
    sharedfilesdialog/linkproxymodel.cpp
)

kde4_add_ui_files(kcm_bluedeviltransfer_PART_SRCS_UI transfer.ui sharedfilesdialog/sharedfiles.ui)

kde4_add_kcfg_files(kcm_bluedeviltransfer_PART_SRCS
                    ../settings/filereceiversettings.kcfgc
                    ../settings/globalsettings.kcfgc)

kde4_add_kcfg_files(kcm_bluedevildevices_PART_SRCS ../settings/globalsettings.kcfgc)

kde4_add_kcfg_files(kcm_bluedeviladapters_PART_SRCS ../settings/globalsettings.kcfgc)

qt4_add_dbus_interface(kcm_bluedeviltransfer_PART_SRCS
    org.kde.BlueDevil.Service.xml
    bluedevil_service)

kde4_add_plugin(kcm_bluedevildevices ${kcm_bluedevildevices_PART_SRCS})
kde4_add_plugin(kcm_bluedeviladapters ${kcm_bluedeviladapters_PART_SRCS})
kde4_add_plugin(kcm_bluedeviltransfer ${kcm_bluedeviltransfer_PART_SRCS} ${kcm_bluedeviltransfer_PART_SRCS_UI})

qt4_automoc(${kcm_bluedevildevices}
            ${kcm_bluedeviladapters}
            ${kcm_bluedeviltransfer})

target_link_libraries(kcm_bluedevildevices ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${LibBlueDevil_LIBRARIES})
target_link_libraries(kcm_bluedeviladapters ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${LibBlueDevil_LIBRARIES})
target_link_libraries(kcm_bluedeviltransfer ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${LibBlueDevil_LIBRARIES})

install(TARGETS kcm_bluedevildevices
                kcm_bluedeviladapters
                kcm_bluedeviltransfer DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES bluedevildevices.desktop
              bluedeviladapters.desktop
              bluedeviltransfer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
