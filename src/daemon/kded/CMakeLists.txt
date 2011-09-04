set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    agentlistener.cpp
    agentlistenerworker.cpp
)

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

set(receiver_SRCS
    receivejob.cpp
    main.cpp
    obexdagent.cpp
    filetransfer.cpp
)

qt4_add_dbus_interface(receiver_SRCS org.openobex.xml obexd_interface)
qt4_add_dbus_interface(receiver_SRCS org.openobex.transfer.xml obexd_transfer)
kde4_add_kcfg_files(receiver_SRCS ../../settings/filereceiversettings.kcfgc)

kde4_add_executable(bluedevil-receiver ${receiver_SRCS})

target_link_libraries(bluedevil-receiver ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${LibBlueDevil_LIBRARIES})