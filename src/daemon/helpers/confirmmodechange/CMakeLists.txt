set(confirmmodechange_SRCS
    main.cpp
    confirmmodechange.cpp
)

kde4_add_executable(bluedevil-confirmmodechange ${confirmmodechange_SRCS})

target_link_libraries(bluedevil-confirmmodechange
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevil-confirmmodechange
        DESTINATION ${LIBEXEC_INSTALL_DIR})
