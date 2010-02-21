include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)
 
# We add our source code here
set(confirmmodechange_SRCS 
	main.cpp
	confirmmodechange.cpp
)

kde4_add_executable(bluedevil-confirmmodechange ${confirmmodechange_SRCS})

target_link_libraries(bluedevil-confirmmodechange
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY})
 
install(TARGETS bluedevil-confirmmodechange
        DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES bluedevil-confirmmodechange.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil-confirmmodechange)
