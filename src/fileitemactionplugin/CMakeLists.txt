kde4_add_plugin(bluetoothfiletiemaction sendfileitemaction.cpp)
target_link_libraries(bluetoothfiletiemaction ${KDE4_KIO_LIBS} ${LibBlueDevil_LIBRARIES})
install(TARGETS bluetoothfiletiemaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevilsendfile.desktop DESTINATION ${SERVICES_INSTALL_DIR})