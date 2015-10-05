set(DEFAULT_TVNC_USEPAM 1)
option(TVNC_USEPAM "Include PAM authentication support in the TurboVNC Server"
	${DEFAULT_TVNC_USEPAM})

include_directories(. ../../fb ../../mi ../../miext/damage ../../os
	../../randr ../../render ${CMAKE_SOURCE_DIR}/common/rfb)

add_definitions(${ServerOSDefines})
set(PAMSRC "")
if(TVNC_USEPAM)
	add_definitions(-DXVNC_AuthPAM)
	set(PAMSRC authpam.c)
endif()

add_library(vnc STATIC
	auth.c
	cmap.c
	corre.c
	cursor.c
	cutpaste.c
	dispcur.c
	draw.c
	flowcontrol.c
	hextile.c
	httpd.c
	init.c
	input-xkb.c
	kbdptr.c
	randr.c
	rfbserver.c
	rre.c
	sockets.c
	sprite.c
	stats.c
	tight.c
	translate.c
	vncextinit.c
	zlib.c
	zrle.c
	zrleoutstream.c
	zrlepalettehelper.c
	${PAMSRC})

# The VNC extension code has a lot of type puns.  Disable strict aliasing just
# to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set_source_files_properties(vncextinit.c PROPERTIES COMPILE_FLAGS
		-fno-strict-aliasing)
endif()
