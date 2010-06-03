add_subdirectory(daemon)
add_subdirectory(kcmodule)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)
