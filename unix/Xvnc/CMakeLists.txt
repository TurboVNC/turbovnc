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
elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
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

include(FindPkgConfig)
pkg_check_modules(X11_Xfont2 REQUIRED xfont2)
pkg_check_modules(X11_Pixman REQUIRED pixman-1)
include_directories(${X11_X11_INCLUDE_PATH} ${X11_Xau_INCLUDE_PATH}
	${X11_Xdmcp_INCLUDE_PATH} ${X11_Xkbfile_INCLUDE_PATH}
	${X11_Xfont2_INCLUDEDIR} ${X11_Pixman_INCLUDEDIR}/pixman-1)

option(TVNC_ZLIBNG
	"Build the TurboVNC Server against the SIMD-accelerated zlib-ng implementation in the TurboVNC source tree rather than the system-supplied zlib implementation"
	ON)
boolean_number(TVNC_ZLIBNG)
report_option(TVNC_ZLIBNG "zlib-ng")
if(NOT TVNC_ZLIBNG)
	find_package(ZLIB REQUIRED)
else()
	set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib-ng)
	set(ZLIB_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib/zlib-ng/libz.a)
endif()
include_directories(${ZLIB_INCLUDE_DIRS})

configure_file(include/tvnc_version.h.in
	programs/Xserver/include/tvnc_version.h)

option(TVNC_DRI3
	"Include DRI3 extension in Xvnc (enables GPU acceleration when using open source drivers)"
	ON)
boolean_number(TVNC_DRI3)
boolean_number(TVNC_DRI3 PARENT_SCOPE)
report_option(TVNC_DRI3 "DRI3 extension")

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

add_subdirectory(lib)
add_subdirectory(programs/Xserver)
