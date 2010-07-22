project(obextest)

set(obextest_SRCS
    main.cpp
    obextest.cpp)

QT4_ADD_DBUS_INTERFACE(obextest_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/manager.xml obexftpmanager)
QT4_ADD_DBUS_INTERFACE(obextest_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/session.xml obexftpsession)
set(obexftpfiletransfer_xml ${CMAKE_CURRENT_SOURCE_DIR}/obexftp/filetransfer.xml)
set_source_files_properties(${obexftpfiletransfer_xml} PROPERTIES INCLUDE "types.h")

kde4_add_executable(obextest ${obextest_SRCS})

target_link_libraries(obextest
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
)

QT4_AUTOMOC(${kio_obexftp_SRCS})
