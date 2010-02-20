include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)
 
# We add our source code here
set(requestpin_SRCS 
	main.cpp
	requestpin.cpp
)

kde4_add_executable(bluedevil-requestpin ${requestpin_SRCS})

target_link_libraries(bluedevil-requestpin
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY})
 
install(TARGETS bluedevil-requestpin
        DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES bluedevil-requestpin.notifyrc
       DESTINATION ${DATA_INSTALL_DIR}/bluedevil)
