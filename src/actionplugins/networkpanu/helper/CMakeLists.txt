include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(networkpanuhelper_SRCS
    main.cpp
    networkpanuhelper.cpp
)

kde4_add_executable(bluedevil-network-panu ${networkpanuhelper_SRCS})

target_link_libraries(bluedevil-network-panu bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES} bluedevil)

install(TARGETS bluedevil-network-panu DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-network-panu.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
