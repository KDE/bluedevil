project(obextest)

set(obextest_SRCS
    main.cpp
    obextest.cpp)

set(SENDER_PLUGIN_PATH ${CMAKE_SOURCE_DIR}/src/actionplugins/sendfile/helper/)
qt4_add_dbus_interface(obextest_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.Client1.xml obexd_client)
qt4_add_dbus_interface(obextest_SRCS ${SENDER_PLUGIN_PATH}org.bluez.obex.ObjectPush1.xml obexd_push)

set(SUPER_FILE ${CMAKE_SOURCE_DIR}/src/kio/obexftp/obexftpkded/obexftp/org.bluez.obex.FileTransfer1.xml)
# set_source_files_properties(${SUPER_FILE} PROPERTIES INCLUDE ${CMAKE_SOURCE_DIR}/src/daemon/obexftpkded/obexftp/filetransfertypes.h)
set_source_files_properties(${SUPER_FILE} PROPERTIES INCLUDE "types.h")
qt4_add_dbus_interface(obextest_SRCS ${SUPER_FILE} obexd_file_transfer)

kde4_add_executable(obextest ${obextest_SRCS})

target_link_libraries(obextest
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${LibBlueDevil_LIBRARIES}
)

QT4_AUTOMOC(${kio_obexftp_SRCS})
