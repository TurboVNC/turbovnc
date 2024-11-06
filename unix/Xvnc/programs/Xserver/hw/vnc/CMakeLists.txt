set(DEFAULT_TVNC_USEPAM 1)
option(TVNC_USEPAM "Include PAM authentication support in the TurboVNC Server"
	${DEFAULT_TVNC_USEPAM})

include_directories(. ../../fb ../../mi ../../os ../../randr ../../render
	${CMAKE_SOURCE_DIR}/common/rfb ${TJPEG_INCLUDE_DIR})

add_definitions(${ServerOSDefines})
set(PAMSRC "")
if(TVNC_USEPAM)
	if(NOT APPLE)
		check_include_files(security/pam_appl.h HAVE_PAM_APPL_H)
		if(NOT HAVE_PAM_APPL_H)
			message(FATAL_ERROR "Could not find PAM headers")
		endif()
	endif()
	add_definitions(-DXVNC_AuthPAM)
	set(PAMSRC authpam.c)
endif()

set(NVCTRLSRC "")
if(TVNC_NVCONTROL)
	set(NVCTRLSRC nvctrlext.c)
	include_directories(${CMAKE_SOURCE_DIR}/unix/libXNVCtrl ${X11_INCLUDE_DIR})
endif()

set(DRI3SRC "")
if(TVNC_DRI3)
	set(DRI3SRC dri3.c)
	include_directories(../../dri3 ../../miext/sync)
endif()

set(TVNC_USETLS_DESC
	"Implement server-side TLS encryption using the specified library [Options: OpenSSL (default), None]")
set(TVNC_USETLS "OpenSSL" CACHE STRING ${TVNC_USETLS_DESC})
set_property(CACHE TVNC_USETLS PROPERTY STRINGS OpenSSL None)
string(TOLOWER "${TVNC_USETLS}" TVNC_USETLS)

if(TVNC_USETLS STREQUAL "openssl")
	message(STATUS "Enabling TLS encryption using OpenSSL")
	find_package(OpenSSL REQUIRED)
	set(RFBSSLSRC rfbssl_openssl.c)
	add_definitions(-DUSETLS)
	set(DEFAULT_TVNC_DLOPENSSL 1)
	option(TVNC_DLOPENSSL "Load OpenSSL using dlopen()/dlsym() instead of linking directly with it"
		${DEFAULT_TVNC_DLOPENSSL})
	if(TVNC_DLOPENSSL)
		add_definitions(-DDLOPENSSL)
	endif()
else()
	message(STATUS "Disabling TLS encryption")
endif()

check_function_exists(strsep HAVE_STRSEP)
if(NOT HAVE_STRSEP)
	set(STRSEPSRC strsep.c)
else()
	add_definitions(-DHAVE_STRSEP)
endif()

add_library(vnc STATIC
	auth.c
	base64.c
	cmap.c
	corre.c
	cursor.c
	cutpaste.c
	dispcur.c
	draw.c
	${DRI3SRC}
	flowcontrol.c
	hextile.c
	init.c
	input-xkb.c
	kbdptr.c
	qnum_to_xorgevdev.c
	qnum_to_xorgkbd.c
	randr.c
	rfbscreen.c
	rfbserver.c
	rre.c
	sockets.c
	sprite.c
	stats.c
	${STRSEPSRC}
	tight.c
	translate.c
	vncextinit.c
	websockets.c
	ws_decode.c
	zlib.c
	zrle.c
	zrleoutstream.c
	zrlepalettehelper.c
	${PAMSRC}
	${NVCTRLSRC}
	${RFBSSLSRC})

if(TVNC_DRI3)
	target_link_libraries(vnc PkgConfig::DRM)
endif()
if(TVNC_USETLS STREQUAL "openssl" AND NOT TVNC_DLOPENSSL)
	target_link_libraries(vnc OpenSSL::SSL)
endif()
