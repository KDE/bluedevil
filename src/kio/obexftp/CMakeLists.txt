project(kio_obexftp)

set(kio_obexftp_PART_SRCS
   qobexftp.cpp
   kio_obexftp.cpp)

kde4_add_plugin(kio_obexftp ${kio_obexftp_PART_SRCS})

target_link_libraries(kio_obexftp
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTTEST_LIBRARY}
  obexftp
)

install(TARGETS kio_obexftp DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexftp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
