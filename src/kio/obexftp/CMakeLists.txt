project(kio_obexftp)

add_subdirectory(obexftpkded)

set(kio_obexftp_SRCS
    kio_obexftp.cpp
   )

set(FILE_TRANSFER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/kded_obexftp.xml)
set_source_files_properties(${FILE_TRANSFER_FILE} PROPERTIES INCLUDE "obexftp_type.h")
QT4_ADD_DBUS_INTERFACE(kio_obexftp_SRCS ${FILE_TRANSFER_FILE} kdedobexftp)

set(obexftpfiletransfer_xml ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/filetransfer.xml)

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
