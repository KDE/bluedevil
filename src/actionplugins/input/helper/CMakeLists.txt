include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(inputhelper_SRCS
    main.cpp
    inputhelper.cpp
    ../../../wizard/wizardagent.cpp
)

kde4_add_executable(bluedevil-input ${inputhelper_SRCS})

target_link_libraries(bluedevil-input bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES} bluedevil)

install(TARGETS bluedevil-input DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-input.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
