set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    bluezagent.cpp
)

qt4_add_dbus_interface(kded_bluedevil_SRCS
    ../helpers/filereceiver/org.kde.BlueDevil.Service.xml
    bluedevil_service_interface)

kde4_add_kcfg_files(kded_bluedevil_SRCS ../../settings/filereceiversettings.kcfgc)

kde4_add_plugin(kded_bluedevil
    ${kded_bluedevil_SRCS}
)

target_link_libraries(kded_bluedevil
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS kded_bluedevil DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

