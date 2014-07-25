project(kio_obexftp)

add_subdirectory(daemon)

set(kio_obexftp_SRCS
    kio_obexftp.cpp
    transferfilejob.cpp
    debug_p.cpp
   )

qt5_add_dbus_interface(kio_obexftp_SRCS kded_obexftp.xml kdedobexftp)

add_library(kio_obexftp MODULE ${kio_obexftp_SRCS})

target_link_libraries(kio_obexftp
    Qt5::Core
    Qt5::DBus
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    KF5::QBluez
)

install(TARGETS kio_obexftp DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexftp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
