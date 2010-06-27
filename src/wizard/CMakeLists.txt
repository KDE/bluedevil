include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_BUILD_DIR}
   ${KDE4_INCLUDES}
)

add_subdirectory(plugins)

set(wizard_SRCS
    main.cpp
    bluewizard.cpp
    wizardagent.cpp
    pages/introductionpage.cpp
    pages/discoverpage.cpp
    pages/pinpage.cpp
    pages/pairingpage.cpp
    pages/servicespage.cpp
    pages/serviceoption.cpp
)

kde4_add_ui_files(wizard_SRCS
            pages/introduction.ui
            pages/discover.ui
            pages/pin.ui
            pages/pairing.ui
            pages/services.ui
            pages/serviceOption.ui
)

qt4_add_dbus_interface(wizard_SRCS
    pages/org.bluez.Audio.xml
    audio_interface)

qt4_add_dbus_interface(wizard_SRCS
    pages/org.bluez.Input.xml
    input_interface)

kde4_add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard bluedevilservice
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY} bluedevil)

install(TARGETS bluedevil-wizard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES pin-code-database.xml DESTINATION ${DATA_INSTALL_DIR}/bluedevil)