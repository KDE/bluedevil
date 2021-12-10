set(sendfilehelper_SRCS
    main.cpp
    discoverwidget.cpp
    sendfilewizard.cpp
    sendfilesjob.cpp

    pages/selectdeviceandfilespage.cpp
    pages/selectdevicepage.cpp
    pages/selectfilespage.cpp
    pages/connectingpage.cpp
    pages/failpage.cpp
)

ecm_qt_declare_logging_category(sendfilehelper_SRCS HEADER bluedevil_sendfile.h IDENTIFIER BLUEDEVIL_SENDFILE_LOG CATEGORY_NAME
    org.kde.plasma.bluedevil.sendfile DESCRIPTION "BlueDevil (send file)" EXPORT BLUEDEVIL)

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
