set(authorize_SRCS
    main.cpp
    authorize.cpp
)

kde4_add_executable(bluedevil-authorize ${authorize_SRCS})

target_link_libraries(bluedevil-authorize
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-authorize
        DESTINATION ${LIBEXEC_INSTALL_DIR})
