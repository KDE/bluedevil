add_subdirectory(helper)
set(bluedevilnetworkpanuactionplugin_SRCS networkpanu.cpp)

kde4_add_plugin(bluedevilnetworkpanuactionplugin ${bluedevilnetworkpanuactionplugin_SRCS})

target_link_libraries(bluedevilnetworkpanuactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS bluedevilnetworkpanuactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-network-panu.desktop DESTINATION ${SERVICES_INSTALL_DIR})
