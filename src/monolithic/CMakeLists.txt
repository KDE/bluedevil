set(monolithic_SRCS
    main.cpp
    monolithic.cpp
)

qt4_add_dbus_interface(monolithic_SRCS org.bluez.Audio.xml audio_interface)

kde4_add_executable(bluedevil-monolithic ${monolithic_SRCS})

target_link_libraries(bluedevil-monolithic ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-monolithic ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-monolithic.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
