include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(networkdunhelper_SRCS
    main.cpp
    networkdunhelper.cpp
)

kde4_add_executable(bluedevil-network-dun ${networkdunhelper_SRCS})

target_link_libraries(bluedevil-network-dun bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES} bluedevil)

install(TARGETS bluedevil-network-dun DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-network-dun.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
