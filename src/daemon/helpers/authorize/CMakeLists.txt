set(authorize_SRCS
    main.cpp
    authorize.cpp
)

add_executable(bluedevil-authorize ${authorize_SRCS})

target_link_libraries(bluedevil-authorize
    KF5::KDELibs4Support
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-authorize
        DESTINATION ${LIBEXEC_INSTALL_DIR})
