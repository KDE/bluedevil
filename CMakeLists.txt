find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include (KDE4Defaults)

add_subdirectory(kded)
add_subdirectory(helpers)
add_subdirectory(helper)

install(FILES bluedevil.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)