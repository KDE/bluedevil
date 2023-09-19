ecm_add_qml_module(bluetoothplugin URI org.kde.plasma.private.bluetooth GENERATE_PLUGIN_SOURCE)

target_sources(bluetoothplugin PRIVATE
    devicesproxymodel.cpp
    launchapp.cpp
    notify.cpp)

target_link_libraries(bluetoothplugin PRIVATE
    Qt::Core
    Qt::Qml
    KF6::Notifications
    KF6::JobWidgets
    KF6::KIOGui
    KF6::BluezQt
    KF6::Service)

ecm_finalize_qml_module(bluetoothplugin)
