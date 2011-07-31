add_subdirectory(helper)
set(bluedevilnetworkdunactionplugin_SRCS networkdun.cpp)

kde4_add_plugin(bluedevilnetworkdunactionplugin ${bluedevilnetworkdunactionplugin_SRCS})

target_link_libraries(bluedevilnetworkdunactionplugin ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${LibBlueDevil_LIBRARIES} bluedevilaction)

install(TARGETS bluedevilnetworkdunactionplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES bluedevil-network-dun.desktop DESTINATION ${SERVICES_INSTALL_DIR})
