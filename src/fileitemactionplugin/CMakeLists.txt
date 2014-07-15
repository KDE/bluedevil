add_library(bluetoothfileitemaction MODULE sendfileitemaction.cpp)
kservice_desktop_to_json(bluetoothfileitemaction bluedevilsendfile.desktop)
target_link_libraries(bluetoothfileitemaction
    Qt5::Widgets
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::QBluez)
install(TARGETS bluetoothfileitemaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevilsendfile.desktop DESTINATION ${SERVICES_INSTALL_DIR})
