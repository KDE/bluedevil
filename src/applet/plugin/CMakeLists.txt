ecm_add_qml_module(bluetoothplugin URI org.kde.plasma.private.bluetooth GENERATE_PLUGIN_SOURCE)

# Reuse applet's domain, because the only strings here so far were moved from it
target_compile_definitions(bluetoothplugin PRIVATE TRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.bluetooth")

target_sources(bluetoothplugin PRIVATE
    devicesproxymodel.cpp
    launchapp.cpp
    notify.cpp
)

target_link_libraries(bluetoothplugin PRIVATE
    Qt::Core
    Qt::Qml

    KF6::BluezQt
    KF6::I18n
    KF6::JobWidgets
    KF6::KIOGui
    KF6::Notifications
    KF6::Service
)

ecm_finalize_qml_module(bluetoothplugin)
