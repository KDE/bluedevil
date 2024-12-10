set(sendfilehelper_SRCS
    discoverwidget.cpp discoverwidget.h
    main.cpp
    sendfilesjob.cpp sendfilesjob.h
    sendfilewizard.cpp sendfilewizard.h

    pages/connectingpage.cpp pages/connectingpage.h
    pages/failpage.cpp pages/failpage.h
    pages/selectdeviceandfilespage.cpp pages/selectdeviceandfilespage.h
    pages/selectdevicepage.cpp pages/selectdevicepage.h
    pages/selectfilespage.cpp pages/selectfilespage.h
)

ecm_qt_declare_logging_category(sendfilehelper_SRCS
    HEADER bluedevil_sendfile.h
    IDENTIFIER BLUEDEVIL_SENDFILE_LOG
    CATEGORY_NAME org.kde.plasma.bluedevil.sendfile
    DESCRIPTION "BlueDevil (send file)"
    EXPORT BLUEDEVIL
)

ki18n_wrap_ui(sendfilehelper_SRCS
    discover.ui
    pages/connecting.ui
    pages/failpage.ui
    pages/selectfilediscover.ui
)

add_executable(bluedevil-sendfile ${sendfilehelper_SRCS})

target_link_libraries(bluedevil-sendfile
    Qt::DBus
    Qt::Widgets

    KF6::BluezQt
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::KIOFileWidgets
    KF6::WidgetsAddons
    KF6::WindowSystem
)

install(TARGETS bluedevil-sendfile DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilsendfile.desktop DESTINATION ${KDE_INSTALL_APPDIR})
