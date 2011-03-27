set(wizard_SRCS
    main.cpp
    bluewizard.cpp
    wizardagent.cpp

    pages/discoverpage.cpp
    pages/servicespage.cpp
    pages/serviceoption.cpp
    pages/nopairing.cpp
    pages/legacypairing.cpp
    pages/legacypairingdatabase.cpp
    pages/keyboardpairing.cpp
    pages/ssppairing.cpp
    pages/fail.cpp
)

kde4_add_ui_files(wizard_SRCS
            pages/discover.ui
            pages/services.ui
            pages/serviceOption.ui
            pages/nopairing.ui
            pages/legacypairing.ui
            pages/keyboardpairing.ui
            pages/ssppairing.ui
            pages/fail.ui
)

kde4_add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-wizard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-wizard.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES pin-code-database.xml DESTINATION ${DATA_INSTALL_DIR}/bluedevilwizard)
