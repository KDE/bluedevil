set(requestpin_SRCS
    main.cpp
    requestpin.cpp
)

ki18n_wrap_ui(requestpin_UI
    dialogwidget.ui)

add_executable(bluedevil-requestpin ${requestpin_SRCS} ${requestpin_UI})

target_link_libraries(bluedevil-requestpin
    Qt5::Core
    KF5::I18n
    KF5::Notifications
    KF5::WindowSystem)

install(TARGETS bluedevil-requestpin
        DESTINATION ${LIBEXEC_INSTALL_DIR})
