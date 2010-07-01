include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BUILD_DIR}
    ${KDE4_INCLUDES}
)

set(monolitic_SRCS
    main.cpp
    monolitic.cpp
)

kde4_add_executable(bluedevil-monolitic ${monolitic_SRCS})

target_link_libraries(bluedevil-monolitic
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY} bluedevil)

install(TARGETS bluedevil-monolitic ${INSTALL_TARGETS_DEFAULT_ARGS})
