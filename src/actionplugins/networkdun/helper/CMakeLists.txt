include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(networkhelper_SRCS
    main.cpp
    networkhelper.cpp
)

kde4_add_executable(bluedevil-network ${networkhelper_SRCS})

target_link_libraries(bluedevil-network bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES} bluedevil)

install(TARGETS bluedevil-network DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-network.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
