#
# Setup
#

cmake_minimum_required(VERSION 2.8.11)
# Shut up CMake 3.1 and later
if(POLICY CMP0053)
  cmake_policy(SET CMP0053 OLD)
endif()

if(UNIX)
	set(CMAKE_INSTALL_PREFIX /opt/TurboVNC CACHE PATH
		"Install path prefix, prepended onto install directories.")
endif()

project(TurboVNC NONE)
set(VERSION 2.1)
set(RCVERSION 2,1,0,0)
set(COPYRIGHT_YEAR "1999-2016")
set(COPYRIGHT "The VirtualGL Project and many others (see README.txt)")
set(URLTEXT "Visit http://www.TurboVNC.org for more information on TurboVNC")

set(USEC 0)

if(APPLE)
	set(DEFAULT_TVNC_BUILDJAVA 1)
elseif(UNIX)
	set(DEFAULT_TVNC_BUILDJAVA 1)
	set(DEFAULT_TVNC_BUILDNATIVE 1)
	option(TVNC_BUILDNATIVE "Build native/X11 support applications and libraries for the Java TurboVNC Viewer"
		${DEFAULT_TVNC_BUILDNATIVE})
else()
	set(DEFAULT_TVNC_BUILDJAVA 0)
	set(DEFAULT_TVNC_BUILDNATIVE 1)
	option(TVNC_BUILDNATIVE "Build native version of the TurboVNC Viewer"
		${DEFAULT_TVNC_BUILDNATIVE})
endif()
option(TVNC_BUILDJAVA "Build Java version of the TurboVNC Viewer"
	${DEFAULT_TVNC_BUILDJAVA})

set(DEFAULT_TVNC_BUILDSERVER 0)
if(UNIX AND NOT APPLE AND NOT CYGWIN)
	set(DEFAULT_TVNC_BUILDSERVER 1)
endif()
option(TVNC_BUILDSERVER "Build TurboVNC server components"
	${DEFAULT_TVNC_BUILDSERVER})

if(TVNC_BUILDNATIVE OR TVNC_BUILDSERVER)
	set(USEC 1)
endif()

if(USEC)
	enable_language(C)

	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()

	message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(UNIX OR (WIN32 AND NOT MINGW))
	string(TIMESTAMP BUILD "%Y%m%d")
else()
	message(FATAL_ERROR "Platform not supported.")
endif()

if(USEC)
	# This only works if building from the command line.  There is currently no
	# way to set a variable's value based on the build type when using the MSVC
	# IDE.
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(BUILD "${BUILD}d")
	endif()
endif()

message(STATUS "VERSION = ${VERSION}, BUILD = ${BUILD}")

message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

set(DEFAULT_TVNC_DOCDIR ${CMAKE_INSTALL_PREFIX}/doc)
if(WIN32)
	set(DEFAULT_TVNC_BINDIR ${CMAKE_INSTALL_PREFIX})
else()
	set(DEFAULT_TVNC_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

set(TVNC_BINDIR ${DEFAULT_TVNC_BINDIR} CACHE PATH
	"Directory into which executables and scripts should be installed (default: ${DEFAULT_TVNC_BINDIR})")
set(TVNC_DOCDIR ${DEFAULT_TVNC_DOCDIR} CACHE PATH
	"Directory into which docs should be installed (default: ${DEFAULT_TVNC_DOCDIR})")

message(STATUS "TVNC_BINDIR = ${TVNC_BINDIR}")
message(STATUS "TVNC_DOCDIR = ${TVNC_DOCDIR}")

# Detect CPU type and word size
if(USEC)
	if(${CMAKE_OSX_ARCHITECTURES} MATCHES "ppc")
		set(CMAKE_SYSTEM_PROCESSOR ppc)
	endif()
	math(EXPR BITS "${CMAKE_SIZEOF_VOID_P} * 8")
	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64"
		OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "amd64"
		OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64"
		OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i[0-9]86")
		if(BITS EQUAL 64)
			set(CPU_TYPE x86_64)
		else()
			set(CPU_TYPE i386)
		endif()
	else()
		string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} CPU_TYPE)
	endif()
	message(STATUS "${BITS}-bit build (${CPU_TYPE})")
endif()
if(TVNC_BUILDNATIVE OR TVNC_BUILDSERVER)
	include(cmakescripts/FindTurboJPEG.cmake)
endif()


#
# Build
#

if(TVNC_BUILDJAVA)
	add_subdirectory(java)
endif()
if(WIN32 AND TVNC_BUILDNATIVE)
	add_subdirectory(win)
endif()
if(UNIX AND (TVNC_BUILDNATIVE OR TVNC_BUILDSERVER))
	add_subdirectory(unix)
endif()
add_subdirectory(doc)


#
# Installation and packaging
#

include(cmakescripts/BuildPackages.cmake)

configure_file("${CMAKE_SOURCE_DIR}/cmakescripts/cmake_uninstall.cmake.in"
  "cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)

install(FILES README.txt DESTINATION .)
