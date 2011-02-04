set(requestpin_SRCS
    main.cpp
    requestpin.cpp
)

kde4_add_ui_files(requestpin_UI
                  dialogWidget.ui)

kde4_add_executable(bluedevil-requestpin ${requestpin_SRCS} ${requestpin_UI})

target_link_libraries(bluedevil-requestpin
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevil-requestpin
        DESTINATION ${LIBEXEC_INSTALL_DIR})
