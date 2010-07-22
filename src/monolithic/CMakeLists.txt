set(monolithic_SRCS
    main.cpp
    monolithic.cpp
)

kde4_add_executable(bluedevil-monolithic ${monolithic_SRCS})

target_link_libraries(bluedevil-monolithic
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-monolithic ${INSTALL_TARGETS_DEFAULT_ARGS})
