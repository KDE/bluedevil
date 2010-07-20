project(kio_obexd)

set(kio_obexd_PART_SRCS
   kio_obexd.cpp)

kde4_add_plugin(kio_obexd ${kio_obexd_PART_SRCS})

target_link_libraries(kio_obexd
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTTEST_LIBRARY}
)

install(TARGETS kio_obexd DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES obexd.protocol DESTINATION ${SERVICES_INSTALL_DIR})
