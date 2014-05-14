set(confirmmodechange_SRCS
    main.cpp
    confirmmodechange.cpp
)

add_executable(bluedevil-confirmmodechange ${confirmmodechange_SRCS})

target_link_libraries(bluedevil-confirmmodechange
    Qt5::Core
    KF5::I18n
    KF5::Notifications)

install(TARGETS bluedevil-confirmmodechange
        DESTINATION ${LIBEXEC_INSTALL_DIR})
