set(requestpin_SRCS
    main.cpp
    requestpin.cpp
)

qt5_wrap_ui(requestpin_UI
    dialogWidget.ui)

add_executable(bluedevil-requestpin ${requestpin_SRCS} ${requestpin_UI})

target_link_libraries(bluedevil-requestpin
    KF5::KDELibs4Support)

install(TARGETS bluedevil-requestpin
        DESTINATION ${LIBEXEC_INSTALL_DIR})
