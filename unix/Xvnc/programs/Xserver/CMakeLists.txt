include_directories(include ../../include/fonts ../../include/extensions)

add_definitions(-DSHAPE -DXINPUT -DRENDER -DRANDR ${ServerExtraDefines})

set(ServerOSDefines -DDDXOSINIT -DSERVER_LOCK -DDDXOSFATALERROR -DDDXTIME)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(ServerOSDefines ${ServerOSDefines} -DPART_NET)
endif()
set(EXT_DEFINES -DMITMISC -DXTEST -DXSYNC -DXCMISC -DMITSHM -DBIGREQS
	-DSCREENSAVER)

add_subdirectory(dix)
add_subdirectory(os)
add_subdirectory(mi)
add_subdirectory(Xext)
add_subdirectory(Xi)
add_subdirectory(randr)
add_subdirectory(render)
add_subdirectory(fb)
add_subdirectory(hw/vnc)

add_executable(Xvnc dummy.c)
set(PAM_LIB "")
if(TVNC_USEPAM)
	set(PAM_LIB pam)
endif()
set(EXTRA_LIB "")
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(EXTRA_LIB "-lsocket -lnsl")
endif()
target_link_libraries(Xvnc Xfont dix os Xau Xdmcp Xfont freetype2 Xi vnc
	vncauth fb xpstubs mi Xext randr render zlib m ${TJPEG_LIBRARY} pthread
	${PAM_LIB} ${EXTRA_LIB})

install(TARGETS Xvnc DESTINATION ${TVNC_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Xserver.man
	DESTINATION ${TVNC_MANDIR}/man1 RENAME Xserver.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Xvnc.man
	DESTINATION ${TVNC_MANDIR}/man1 RENAME Xvnc.1)
