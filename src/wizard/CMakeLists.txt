set(wizard_SRCS
    main.cpp
    bluewizard.cpp
    wizardagent.cpp
    debug_p.cpp

    pages/discoverpage.cpp
    pages/nopairing.cpp
    pages/legacypairing.cpp
    pages/legacypairingdatabase.cpp
    pages/keyboardpairing.cpp
    pages/ssppairing.cpp
    pages/fail.cpp
    pages/success.cpp
)

ki18n_wrap_ui(wizard_SRCS
            pages/discover.ui
            pages/nopairing.ui
            pages/legacypairing.ui
            pages/keyboardpairing.ui
            pages/ssppairing.ui
            pages/fail.ui
            pages/success.ui
)

add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::WidgetsAddons
    KF5::IconThemes
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-wizard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilwizard.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES pin-code-database.xml DESTINATION ${DATA_INSTALL_DIR}/bluedevilwizard)
