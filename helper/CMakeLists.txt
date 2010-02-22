include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

set(helper_SRCS
    main.cpp
    service.cpp
    openobex/serversession.cpp
    openobex/serversessionfiletransfer.cpp
)

qt4_add_dbus_adaptor(helper_SRCS org.kde.BlueDevil.Service.xml service.h Service)

qt4_add_dbus_interface(helper_SRCS org.openobex.ServerSession.xml server_session_interface)

kde4_add_executable(bluedevil-helper ${helper_SRCS})

target_link_libraries(bluedevil-helper
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY}
)

install(TARGETS bluedevil-helper
    DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
)


# for dbus_add_activation_service
include(PkgConfigGetVar)

dbus_add_activation_service(org.kde.BlueDevil.Service.service.in)

# configure_file(org.kde.BlueDevil.Service.cmake
#     ${CMAKE_CURRENT_BINARY_DIR}/org.kde.BlueDevil.Service)
# 
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.BlueDevil.Service
#     DESTINATION ${DBUS_SERVICES_INSTALL_DIR})




add_subdirectory(openobex)