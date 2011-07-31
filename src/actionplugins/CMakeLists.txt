add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(sendfile)
add_subdirectory(networkdun)
add_subdirectory(networkpanu)

kde4_add_library(bluedevilaction SHARED actionplugin.cpp)

target_link_libraries(bluedevilaction ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevilaction ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES actionplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES actionplugin.h DESTINATION include/)
