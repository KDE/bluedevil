add_subdirectory(settings)
add_subdirectory(actionplugins)
add_subdirectory(daemon)
add_subdirectory(kcmodule)
add_subdirectory(kio)
add_subdirectory(wizard)
add_subdirectory(monolithic)
add_subdirectory(fileitemactionplugin)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)

install(FILES bluedevil-mime.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})