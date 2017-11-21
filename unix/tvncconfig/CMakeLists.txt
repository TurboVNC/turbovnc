include_directories(${X11_INCLUDE_DIR})

add_executable(tvncconfig tvncconfig.c vncExt.c)

target_link_libraries(tvncconfig ${X11_LIBRARIES})

install(TARGETS tvncconfig DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES tvncconfig.man DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
	RENAME tvncconfig.1)
