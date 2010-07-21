project(kio_obexftp)

set(kio_obexftp_SRCS
    kio_obexftp.cpp
    agent.cpp)

QT4_ADD_DBUS_INTERFACE(kio_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/manager.xml obexftpmanager)
QT4_ADD_DBUS_INTERFACE(kio_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/client.xml obexftpclient)
QT4_ADD_DBUS_INTERFACE(kio_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/session.xml obexftpsession)
set(obexftpfiletransfer_xml ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/filetransfer.xml)
set_source_files_properties(${obexftpfiletransfer_xml} PROPERTIES INCLUDE "types.h")
QT4_ADD_DBUS_INTERFACE(kio_obexftp_SRCS ${obexftpfiletransfer_xml} obexftpfiletransfer)
QT4_ADD_DBUS_ADAPTOR(kio_obexftp_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/org.openobex.Agent.xml
                     agent.h Agent)

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
