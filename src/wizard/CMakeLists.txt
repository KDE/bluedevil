include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_BUILD_DIR}
   ${KDE4_INCLUDES}
)

set(wizard_SRCS
    main.cpp
    wizard.cpp
    wizardagent.cpp
    pages/introductionpage.cpp
    pages/discoverpage.cpp
    pages/pinpage.cpp
)

kde4_add_ui_files(wizard_SRCS
            pages/introduction.ui
            pages/discover.ui
            pages/pin.ui)

kde4_add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY} bluedevil)

install(TARGETS bluedevil-wizard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES pin-code-database.xml DESTINATION ${DATA_INSTALL_DIR}/bluedevil)
