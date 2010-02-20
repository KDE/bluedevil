include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${KDE4WORKSPACE_INCLUDE_DIR})

set(kded_bluedevil_SRCS
    BlueDevilDaemon.cpp
    agentlistener.cpp
    agentlistenerworker.cpp
    openobex/server.cpp
)

qt4_add_dbus_interface(kded_bluedevil_SRCS
    org.openobex.Manager.xml
    manager_interface)

qt4_add_dbus_interface(kded_bluedevil_SRCS
    org.openobex.Server.xml
    server_interface)

kde4_add_plugin(kded_bluedevil
    ${kded_bluedevil_SRCS}
)

target_link_libraries(kded_bluedevil
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY}
    ${KDE4_KUTILS_LIBS}
)

install(TARGETS kded_bluedevil DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
install(FILES org.openobex.Manager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES org.openobex.Session.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
# add_subdirectory(openobex)
