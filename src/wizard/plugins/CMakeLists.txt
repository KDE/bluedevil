add_subdirectory(input)

kde4_add_library(bluedevilservice SHARED serviceplugin.cpp)

target_link_libraries(bluedevilservice ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevilservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES serviceplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES serviceplugin.h DESTINATION include/)