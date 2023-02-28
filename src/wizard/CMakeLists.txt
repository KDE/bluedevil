set(wizard_SRCS
    main.cpp
    bluewizard.cpp
    wizardagent.cpp
    pages/discover.cpp
    pages/connect.cpp
    pages/pairing.cpp
    pages/fail.cpp
    pages/success.cpp
)

ecm_qt_declare_logging_category(wizard_SRCS
    HEADER bluedevil_wizard.h
    IDENTIFIER BLUEDEVIL_WIZARD_LOG
    CATEGORY_NAME org.kde.plasma.bluedevil.wizard
    DESCRIPTION "BlueDevil (wizard)"
    EXPORT BLUEDEVIL
)

ki18n_wrap_ui(wizard_SRCS
    pages/discover.ui
    pages/connect.ui
    pages/pairing.ui
    pages/fail.ui
    pages/success.ui
)

add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard
    Qt::Widgets
    Qt::DBus
    KF6::I18n
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::Notifications
    KF6::WidgetsAddons
    KF6::WindowSystem
    KF6::BluezQt
)

install(TARGETS bluedevil-wizard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilwizard.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES pin-code-database.xml DESTINATION ${KDE_INSTALL_DATADIR}/bluedevilwizard)
