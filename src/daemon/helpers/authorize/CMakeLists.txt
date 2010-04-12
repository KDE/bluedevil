include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(authorize_SRCS
    main.cpp
    authorize.cpp
)

kde4_add_executable(bluedevil-authorize ${authorize_SRCS})

target_link_libraries(bluedevil-authorize
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY})

install(TARGETS bluedevil-authorize
        DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
