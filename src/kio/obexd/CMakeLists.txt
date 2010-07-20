project(kio_obexd)

set(kio_obexd_SRCS
    kio_obexd.cpp)

QT4_ADD_DBUS_INTERFACE(kio_obexd_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexd/manager.xml obexdmanager)
QT4_ADD_DBUS_INTERFACE(kio_obexd_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/obexd/client.xml obexdclient)

kde4_add_plugin(kio_obexd ${kio_obexd_SRCS})

target_link_libraries(kio_obexd
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
)

QT4_AUTOMOC(${kio_obexd_SRCS})

install(TARGETS kio_obexd DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexd.protocol DESTINATION ${SERVICES_INSTALL_DIR})
