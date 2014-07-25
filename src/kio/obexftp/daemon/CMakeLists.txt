set(kded_obexftp_SRCS
    ObexFtpDaemon.cpp
)

add_library(kded_obexftpdaemon MODULE ${kded_obexftp_SRCS})

kservice_desktop_to_json(kded_obexftpdaemon obexftpdaemon.desktop)

target_link_libraries(kded_obexftpdaemon
    Qt5::Core
    KF5::I18n
    KF5::DBusAddons
    KF5::KIOCore
    KF5::QBluez
)

install(TARGETS kded_obexftpdaemon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES obexftpdaemon.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
