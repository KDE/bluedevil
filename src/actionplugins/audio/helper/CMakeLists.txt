include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(audiohelper_SRCS
    main.cpp
    audiohelper.cpp
)

kde4_add_executable(bluedevil-audio ${audiohelper_SRCS})

target_link_libraries(bluedevil-audio bluedevilaction
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${LibBlueDevil_LIBRARIES} bluedevil)

install(TARGETS bluedevil-audio DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-audio.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
