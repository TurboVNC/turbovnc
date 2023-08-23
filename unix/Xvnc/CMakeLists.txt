include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(CheckSymbolExists)

set(GLIBC_SOURCE "\n
	#include <features.h>\n
	#ifndef __GLIBC__\n
	#error\n
	#endif\n
	int main(void) { return 0; }\n")

check_c_source_compiles("${GLIBC_SOURCE}" FOUND_GLIBC)
if(FOUND_GLIBC)
	add_definitions(-D_GNU_SOURCE)
endif()

set(BSD44SOCKETS_SOURCE "\n
	#include <sys/types.h>\n
	#include <sys/socket.h>\n
	#include <netinet/in.h>\n
	int main(void) {\n
		struct sockaddr_in sa;\n
		sa.sin_len = 0;\n
		return sa.sin_len;\n
	}")
check_c_source_compiles("${BSD44SOCKETS_SOURCE}" FOUND_BSD44SOCKETS)

set(SVR4_SOURCE "\n
	#if !defined(SVR4) && !defined(__svr4__) && !defined(__SVR4)\n
	#error\n
	#endif\n
	int main(void) { return 0; }\n")
check_c_source_compiles("${SVR4_SOURCE}" FOUND_SVR4)
if(FOUND_SVR4)
	add_definitions(-DSVR4)
endif()

check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
	add_definitions(-DHAVE_UNISTD_H)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
	message(STATUS "BSD-like system detected")
	add_definitions(-DCSRG_BASED)
elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
	NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	message(WARNING "TurboVNC Server build has not been tested on this platform.  Chaos may ensue.")
endif()

add_definitions(-DIPv6)

check_symbol_exists(arc4random_buf stdlib.h HAVE_ARC4RANDOM_BUF)
if(HAVE_ARC4RANDOM_BUF)
	add_definitions(-DHAVE_ARC4RANDOM_BUF)
endif()

check_symbol_exists(poll poll.h USE_POLL)
if(USE_POLL)
	add_definitions(-DUSE_POLL)
endif()

check_symbol_exists(reallocarray stdlib.h HAVE_REALLOCARRAY)
if(HAVE_REALLOCARRAY)
	add_definitions(-DHAVE_REALLOCARRAY)
endif()

check_symbol_exists(strcasecmp strings.h HAVE_STRCASECMP)
if(HAVE_STRCASECMP)
	add_definitions(-DHAVE_STRCASECMP)
endif()

check_symbol_exists(strlcat string.h HAVE_STRLCAT)
if(HAVE_STRLCAT)
	add_definitions(-DHAVE_STRLCAT)
endif()

check_symbol_exists(strlcpy string.h HAVE_STRLCPY)
if(HAVE_STRLCPY)
	add_definitions(-DHAVE_STRLCPY)
endif()

option(TVNC_SYSTEMX11
	"Build the TurboVNC Server against the system-supplied X11 and OpenGL headers and libraries rather than the in-tree versions.  This will probably fail unless the system has xorg-server 1.19.x or later."
	OFF)
boolean_number(TVNC_SYSTEMX11)
report_option(TVNC_SYSTEMX11 "System X11 headers/libs")

if(NOT TVNC_SYSTEMX11)
	function(copy_X_header file dir)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
		file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
	endfunction()
endif()

if(TVNC_SYSTEMX11)
	include(FindPkgConfig)
	pkg_check_modules(X11_Xfont2 REQUIRED xfont2)
	pkg_check_modules(X11_Fontenc REQUIRED fontenc)
	pkg_check_modules(X11_Pixman REQUIRED pixman-1)
	include_directories(${X11_X11_INCLUDE_PATH} ${X11_Xau_INCLUDE_PATH}
		${X11_Xdmcp_INCLUDE_PATH} ${X11_Xkbfile_INCLUDE_PATH}
		${X11_Xfont2_INCLUDEDIR} ${X11_Pixman_INCLUDEDIR}/pixman-1)
else()
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/X_include)
	set(X11_Xau_LIB Xau)
	set(X11_Xdmcp_LIB Xdmcp)
	set(X11_Xfont2_LDFLAGS Xfont2)
	set(X11_Fontenc_LDFLAGS fontenc)
	set(X11_Pixman_LDFLAGS pixman)
endif()

set(USE_INTEL_ZLIB 0)
if((CPU_TYPE STREQUAL "x86_64" OR CPU_TYPE STREQUAL "i386") AND
	NOT TVNC_SYSTEMLIBS AND NOT CMAKE_C_COMPILER_ID MATCHES SunPro)
	set(USE_INTEL_ZLIB 1)
endif()
if(NOT USE_INTEL_ZLIB)
	include(FindZLIB)
else()
	set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib)
	set(ZLIB_LIBRARIES zlib)
endif()
if(TVNC_SYSTEMLIBS)
	include(FindBZip2)
else()
	set(BZIP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/bzip2)
	set(BZIP2_LIBRARIES bzip2)
endif()
include_directories(${ZLIB_INCLUDE_DIRS})

configure_file(include/tvnc_version.h.in
	programs/Xserver/include/tvnc_version.h)

if(NOT APPLE)
	option(TVNC_GLX
		"Include GLX extension in Xvnc (causes Xvnc to depend on the system's libX11 and libXext)"
		ON)
	boolean_number(TVNC_GLX)
	report_option(TVNC_GLX "GLX extension")
endif()

if(NOT TVNC_SYSTEMX11)
	add_subdirectory(include)
endif()
if(TVNC_SYSTEMLIBS)
	include(FindFreetype)
else()
	add_subdirectory(extras/freetype2)
	set(FREETYPE_INCLUDE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}/extras/freetype2/include
		${CMAKE_CURRENT_SOURCE_DIR}/extras/freetype2/src/truetype)
	set(FREETYPE_LIBRARIES freetype2)
endif()
if(TVNC_GLX)
	if(TVNC_SYSTEMX11)
		if(POLICY CMP0072)
			cmake_policy(SET CMP0072 OLD)
		endif()
		include(FindOpenGL)
		include_directories(${OPENGL_INCLUDE_DIR})
	else()
		add_subdirectory(extras/Mesa)
	endif()
endif()
add_subdirectory(lib)
add_subdirectory(programs/Xserver)
