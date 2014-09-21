set(DEFAULT_TVNC_USEPAM 1)
option(TVNC_USEPAM "Include PAM authentication support in the TurboVNC Server"
	${DEFAULT_TVNC_USEPAM})

include_directories(. ../../fb ../../mi ../../os ../../render
	${CMAKE_SOURCE_DIR}/common/rfb
	${CMAKE_CURRENT_BINARY_DIR}/../../../../include)

add_definitions(${ServerOSDefines})
set(PAMSRC "")
if(TVNC_USEPAM)
	add_definitions(-DXVNC_AuthPAM)
	set(PAMSRC authpam.c)
endif()

add_library(vnc STATIC
	init.c
	sockets.c
	kbdptr.c
	cmap.c
	draw.c
	cutpaste.c
	vncextinit.c
	dispcur.c
	sprite.c
	rfbserver.c
	translate.c
	httpd.c
	auth.c
	rre.c
	corre.c
	stats.c
	hextile.c
	zlib.c
	tight.c
	zrle.c
	zrleoutstream.c
	zrlepalettehelper.c
	tight.c
	cursor.c
	xistubs.c
	flowcontrol.c
	${PAMSRC})
