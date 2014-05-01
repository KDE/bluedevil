set(confirmmodechange_SRCS
    main.cpp
    confirmmodechange.cpp
)

add_executable(bluedevil-confirmmodechange ${confirmmodechange_SRCS})

target_link_libraries(bluedevil-confirmmodechange
    KF5::KDELibs4Support)

install(TARGETS bluedevil-confirmmodechange
        DESTINATION ${LIBEXEC_INSTALL_DIR})
