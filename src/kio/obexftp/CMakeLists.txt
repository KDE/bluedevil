set(kio_obexftp_SRCS
    kioobexftp.cpp
    transferfilejob.cpp
    debug_p.cpp
   )

set(kded_obexftp.xml ${CMAKE_SOURCE_DIR}/src/interfaces/kded_obexftp.xml)
qt5_add_dbus_interface(kio_obexftp_SRCS ${kded_obexftp.xml} kdedobexftp)

add_library(kio_obexftp MODULE ${kio_obexftp_SRCS})

target_link_libraries(kio_obexftp
    Qt::Core
    Qt::DBus
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    KF5::BluezQt
    Qt::Network
)

install(TARGETS kio_obexftp DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(FILES obexftp.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
