set(requestconfirmation_SRCS
    main.cpp
    requestconfirmation.cpp
)

add_executable(bluedevil-requestconfirmation ${requestconfirmation_SRCS})

target_link_libraries(bluedevil-requestconfirmation
    Qt5::Core
    KF5::I18n
    KF5::Notifications)

install(TARGETS bluedevil-requestconfirmation
        DESTINATION ${LIBEXEC_INSTALL_DIR})
