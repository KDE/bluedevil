add_subdirectory(daemon)
add_subdirectory(wizard)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)