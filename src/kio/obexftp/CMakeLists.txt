project(kio_obexftp)

add_subdirectory(daemon)

set(kio_obexftp_SRCS
    kio_obexftp.cpp
    transferfilejob.cpp
    debug_p.cpp
   )

qt5_add_dbus_interface(kio_obexftp_SRCS kded_obexftp.xml kdedobexftp)

set(FILE_TRANSFER_FILE org.bluez.obex.FileTransfer1.xml)
set_source_files_properties(${FILE_TRANSFER_FILE} PROPERTIES INCLUDE "obexdtypes.h")
qt5_add_dbus_interface(kio_obexftp_SRCS ${FILE_TRANSFER_FILE} obexd_file_transfer)

set(obexftpfiletransfer_xml ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/filetransfer.xml)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/sendfile/)
qt5_add_dbus_interface(kio_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Transfer1.xml obexd_transfer)

add_library(kio_obexftp MODULE ${kio_obexftp_SRCS})

target_link_libraries(kio_obexftp
    Qt5::Core
    Qt5::DBus
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
)

install(TARGETS kio_obexftp DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexftp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
