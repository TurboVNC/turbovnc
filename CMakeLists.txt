#
# Setup
#

cmake_minimum_required(VERSION 2.8.4)

if(UNIX)
	set(CMAKE_INSTALL_PREFIX /opt/TurboVNC CACHE PATH
		"Install path prefix, prepended onto install directories.")
endif()

project(TurboVNC NONE)
set(VERSION 1.2)
set(RCVERSION 1,2,0,0)
set(XVENDORRELEASE 10200000)
set(COPYRIGHT_YEAR "1999-2013")
set(COPYRIGHT "The VirtualGL Project and many others (see README.txt)")
set(URLTEXT "Visit http://www.virtualgl.org for more information on TurboVNC")

if(UNIX)
	execute_process(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE BUILD)
	string(REGEX REPLACE "\n" "" BUILD ${BUILD})
elseif(WIN32 AND NOT MINGW)
	execute_process(COMMAND "wmic.exe" "os" "get" "LocalDateTime" OUTPUT_VARIABLE
		BUILD)
	string(REGEX REPLACE "[^0-9]" "" BUILD "${BUILD}")
	if (BUILD STREQUAL "")
		execute_process(COMMAND "cmd.exe" "/C" "DATE" "/T" OUTPUT_VARIABLE BUILD)
		string(REGEX REPLACE ".*[ ]([0-9]*)[/.]([0-9]*)[/.]([0-9]*).*" "\\3\\2\\1" BUILD "${BUILD}")
	else()
		string(SUBSTRING "${BUILD}" 0 8 BUILD)
	endif()
else()
	message(FATAL_ERROR "Platform not supported.")
endif()
string(REGEX REPLACE "\n" "" BUILD ${BUILD})

if(APPLE)
	set(DEFAULT_TVNC_BUILDJAVA 1)
	set(DEFAULT_TVNC_BUILDNATIVE 0)
else()
	set(DEFAULT_TVNC_BUILDJAVA 0)
	set(DEFAULT_TVNC_BUILDNATIVE 1)
endif()
option(TVNC_BUILDNATIVE "Build native version of the TurboVNC Viewer"
	${DEFAULT_TVNC_BUILDNATIVE})
option(TVNC_BUILDJAVA "Build Java version of the TurboVNC Viewer"
	${DEFAULT_TVNC_BUILDJAVA})

if(TVNC_BUILDNATIVE)
	enable_language(C)

	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()

	message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

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
if(TVNC_BUILDNATIVE)
	math(EXPR BITS "${CMAKE_SIZEOF_VOID_P} * 8")
	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64"
		OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "amd64"
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

	include(cmakescripts/FindTurboJPEG.cmake)
endif()


#
# Build
#

set(DEFAULT_TVNC_BUILDSERVER 0)
if(UNIX AND NOT APPLE AND NOT CYGWIN)
	set(DEFAULT_TVNC_BUILDSERVER 1)
endif()
option(TVNC_BUILDSERVER "Build TurboVNC server components"
	${DEFAULT_TVNC_BUILDSERVER})

if(TVNC_BUILDNATIVE)
	if(WIN32)
		add_subdirectory(win)
	else()
		add_subdirectory(unix)
	endif()
endif()
add_subdirectory(doc)
if(TVNC_BUILDJAVA)
	add_subdirectory(java)
endif()


#
# Installation and packaging
#

include(cmakescripts/BuildPackages.cmake)

configure_file("${CMAKE_SOURCE_DIR}/cmakescripts/cmake_uninstall.cmake.in"
  "cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)

if(TVNC_BUILDNATIVE)
	install(FILES README.txt DESTINATION .)
endif()
