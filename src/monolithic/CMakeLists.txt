set(monolithic_SRCS
    main.cpp
    monolithic.cpp
)

qt5_add_dbus_interface(monolithic_SRCS org.bluez.Audio.xml audio_interface)

add_executable(bluedevil-monolithic ${monolithic_SRCS})

target_link_libraries(bluedevil-monolithic
    KF5::KDELibs4Support
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-monolithic ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-monolithic.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
