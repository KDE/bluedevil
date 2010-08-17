add_subdirectory(helper)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_BUILD_DIR}
   ${KDE4_INCLUDES}
)

set(sendfile_SRCS sendfile.cpp)

kde4_add_plugin(sendfile ${sendfile_SRCS})

target_link_libraries(sendfile ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} bluedevilaction ${LibBlueDevil_LIBRARIES})

install(TARGETS sendfile DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES sendfile.desktop DESTINATION ${SERVICES_INSTALL_DIR})

