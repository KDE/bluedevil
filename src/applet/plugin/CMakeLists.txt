set(bluetoothplugin_SRCS
    devicesproxymodel.cpp
    launchapp.cpp
    notify.cpp
    bluetoothplugin.cpp)

add_library(bluetoothplugin SHARED ${bluetoothplugin_SRCS})
target_link_libraries(bluetoothplugin
    Qt5::Core
    Qt5::Qml
    KF5::Notifications
    KF5::KIOWidgets
    KF5::BluezQt)

install(TARGETS bluetoothplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bluetooth)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bluetooth)
