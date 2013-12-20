project(kio_obexftp)

add_subdirectory(daemon)

set(kio_obexftp_SRCS
    kio_obexftp.cpp
    transferfilejob.cpp
   )

qt4_add_dbus_interface(kio_obexftp_SRCS kded_obexftp.xml kdedobexftp)

set(FILE_TRANSFER_FILE org.bluez.obex.FileTransfer1.xml)
set_source_files_properties(${FILE_TRANSFER_FILE} PROPERTIES INCLUDE "obexdtypes.h")
qt4_add_dbus_interface(kio_obexftp_SRCS ${FILE_TRANSFER_FILE} obexd_file_transfer)

set(obexftpfiletransfer_xml ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/filetransfer.xml)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/sendfile/)
qt4_add_dbus_interface(kio_obexftp_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Transfer1.xml obexd_transfer)

kde4_add_plugin(kio_obexftp ${kio_obexftp_SRCS})

target_link_libraries(kio_obexftp
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
)

QT4_AUTOMOC(${kio_obexftp_SRCS})

install(TARGETS kio_obexftp DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexftp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
