# install the cmake files

# don't glob for the files, since we don't want to
# install e.g. FindLibKNotification-1.cmake
set(cmakeFiles FindCkConnector.cmake
               FindDBus.cmake
               FindGooglegadgets.cmake
               FindLibXKlavier.cmake
               FindPAM.cmake
               FindQEdje.cmake
               FindSensors.cmake
               PkgConfigGetVar.cmake
               UnixAuth.cmake )

set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

install( FILES  ${cmakeFiles} DESTINATION ${module_install_dir} )

