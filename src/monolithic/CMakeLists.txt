set(monolithic_SRCS
    main.cpp
    monolithic.cpp
)

add_executable(bluedevil-monolithic ${monolithic_SRCS})

target_link_libraries(bluedevil-monolithic
    Qt5::Widgets
    KF5::I18n
    KF5::KIOWidgets
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::Notifications
    ${LibBlueDevil_LIBRARIES})

install(TARGETS bluedevil-monolithic ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilmonolithic.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
