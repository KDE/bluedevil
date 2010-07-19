set(monolitic_SRCS
    main.cpp
    monolitic.cpp
)

kde4_add_executable(bluedevil-monolitic ${monolitic_SRCS})

target_link_libraries(bluedevil-monolitic
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-monolitic ${INSTALL_TARGETS_DEFAULT_ARGS})
