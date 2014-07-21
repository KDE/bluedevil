include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
)

set(sendfilehelper_SRCS
    main.cpp
    discoverwidget.cpp
    sendfilewizard.cpp
    pages/selectdeviceandfilespage.cpp
    pages/selectdevicepage.cpp
    pages/selectfilespage.cpp
    pages/connectingpage.cpp
    sendfilesjob.cpp
    debug_p.cpp
)

qt5_wrap_ui(sendfilehelper_SRCS
    pages/connecting.ui
    pages/selectfilediscover.ui
    discover.ui
)

add_executable(bluedevil-sendfile ${sendfilehelper_SRCS})

target_link_libraries(bluedevil-sendfile
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::CoreAddons
    KF5::IconThemes
    KF5::KIOFileWidgets
    KF5::QBluez
)

install(TARGETS bluedevil-sendfile DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-sendfile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
