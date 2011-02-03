include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(sendfilehelper_SRCS
    main.cpp
    discoverwidget.cpp
    sendfilewizard.cpp
    pages/selectdeviceandfilespage.cpp
    pages/selectdevicepage.cpp
    pages/selectfilespage.cpp
    pages/connectingpage.cpp
    obexagent.cpp
    sendfilesjob.cpp
)

kde4_add_ui_files(sendfilehelper_SRCS
    pages/connecting.ui
    pages/selectfilediscover.ui
    discover.ui
)

qt4_add_dbus_interface(sendfilehelper_SRCS org.openobex.Client.xml obex_client)
qt4_add_dbus_interface(sendfilehelper_SRCS org.openobex.Transfer.xml obex_transfer)


kde4_add_executable(bluedevil-sendfile ${sendfilehelper_SRCS})

target_link_libraries(bluedevil-sendfile
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-sendfile DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-sendfile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
