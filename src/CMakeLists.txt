add_subdirectory(settings)
add_subdirectory(sendfile)
add_subdirectory(daemon)
add_subdirectory(kcmodule)
add_subdirectory(kio)
add_subdirectory(wizard)
add_subdirectory(monolithic)

if(NOT KDE_VERSION VERSION_LESS "4.6.0")
  add_subdirectory(fileitemactionplugin)
else(NOT KDE_VERSION VERSION_LESS "4.6.0")
  message(WARNING "Upgrade to kde-4.6 if you want \"Send via bluetooth\" action in Dolphin/Konqueror")
endif(NOT KDE_VERSION VERSION_LESS "4.6.0")

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)

install(FILES bluedevil-mime.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
