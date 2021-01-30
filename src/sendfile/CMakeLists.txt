set(sendfilehelper_SRCS
    main.cpp
    discoverwidget.cpp
    sendfilewizard.cpp
    sendfilesjob.cpp
    debug_p.cpp

    pages/selectdeviceandfilespage.cpp
    pages/selectdevicepage.cpp
    pages/selectfilespage.cpp
    pages/connectingpage.cpp
    pages/failpage.cpp
)

ki18n_wrap_ui(sendfilehelper_SRCS
    discover.ui
    pages/selectfilediscover.ui
    pages/connecting.ui
    pages/failpage.ui
)

add_executable(bluedevil-sendfile ${sendfilehelper_SRCS})

target_link_libraries(bluedevil-sendfile
    Qt::Widgets
    Qt::DBus
    KF5::I18n
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::BluezQt
)

install(TARGETS bluedevil-sendfile DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilsendfile.desktop DESTINATION ${KDE_INSTALL_APPDIR})
