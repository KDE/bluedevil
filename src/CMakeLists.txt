add_subdirectory(daemon)
add_subdirectory(kio)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)
