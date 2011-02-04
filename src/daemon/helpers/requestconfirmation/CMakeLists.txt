set(requestconfirmation_SRCS
    main.cpp
    requestconfirmation.cpp
)

kde4_add_executable(bluedevil-requestconfirmation ${requestconfirmation_SRCS})

target_link_libraries(bluedevil-requestconfirmation
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevil-requestconfirmation
        DESTINATION ${LIBEXEC_INSTALL_DIR})
