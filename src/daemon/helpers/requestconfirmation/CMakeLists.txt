set(requestconfirmation_SRCS
    main.cpp
    requestconfirmation.cpp
)

add_executable(bluedevil-requestconfirmation ${requestconfirmation_SRCS})

target_link_libraries(bluedevil-requestconfirmation
    KF5::KDELibs4Support)

install(TARGETS bluedevil-requestconfirmation
        DESTINATION ${LIBEXEC_INSTALL_DIR})
