# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set(helper_SRCS
    main.cpp
    service.cpp
    openobex/server.cpp
    openobex/serverftp.cpp
    openobex/serversession.cpp
    openobex/filetransferjob.cpp
)

qt4_add_dbus_adaptor(helper_SRCS org.kde.BlueDevil.Service.xml service.h Service)

qt4_add_dbus_interface(helper_SRCS org.openobex.Server.xml server_interface)
qt4_add_dbus_interface(helper_SRCS org.openobex.ServerSession.xml server_session_interface)

kde4_add_kcfg_files(helper_SRCS ../../../settings/filereceiversettings.kcfgc)
kde4_add_executable(bluedevil-helper ${helper_SRCS})

# for dbus_add_activation_service
include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.BlueDevil.Service.service.in)

target_link_libraries(bluedevil-helper
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${LibBlueDevil_LIBRARIES}
)

install(TARGETS bluedevil-helper DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

