include_directories(${X11_INCLUDE_DIR})

add_executable(vncconnect vncconnect.c)

target_link_libraries(vncconnect ${X11_LIBRARIES})

install(TARGETS vncconnect DESTINATION ${TVNC_BINDIR})

install(FILES vncconnect.man DESTINATION ${TVNC_MANDIR}/man1
	RENAME vncconnect.1)
