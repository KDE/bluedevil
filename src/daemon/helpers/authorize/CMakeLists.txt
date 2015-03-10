set(authorize_SRCS
    main.cpp
    authorize.cpp
)

add_executable(bluedevil-authorize ${authorize_SRCS})

target_link_libraries(bluedevil-authorize
    Qt5::Core
    KF5::I18n
    KF5::Notifications
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-authorize
        DESTINATION ${LIBEXEC_INSTALL_DIR})
