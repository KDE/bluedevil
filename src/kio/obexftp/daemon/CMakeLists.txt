set(kded_obexftp_SRCS
    obexftpdaemon.cpp
)

add_library(kded_obexftpdaemon MODULE ${kded_obexftp_SRCS})
set_target_properties(kded_obexftpdaemon PROPERTIES OUTPUT_NAME obexftpdaemon)

kcoreaddons_desktop_to_json(kded_obexftpdaemon obexftpdaemon.desktop)

target_link_libraries(kded_obexftpdaemon
    Qt5::Core
    KF5::I18n
    KF5::DBusAddons
    KF5::KIOCore
    KF5::BluezQt
)

install(TARGETS kded_obexftpdaemon DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
