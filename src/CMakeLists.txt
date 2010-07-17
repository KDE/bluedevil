add_subdirectory(settings)
add_subdirectory(actionplugins)
add_subdirectory(daemon)
add_subdirectory(kcmodule)
add_subdirectory(kio)
add_subdirectory(wizard)
add_subdirectory(monolitic)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)