#
# Setup
#

cmake_minimum_required(VERSION 2.8.4)

if(UNIX)
	set(CMAKE_INSTALL_PREFIX /opt/TurboVNC CACHE PATH
		"Install path prefix, prepended onto install directories.")
endif()

project(TurboVNC C)
set(VERSION 1.1.80)
set(RCVERSION 1,1,80,0)
set(XVENDORRELEASE 10180000)

if(UNIX)
	execute_process(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE BUILD)
elseif(MSVC)
	execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmakescripts/getdate.bat"
		OUTPUT_VARIABLE BUILD)
else()
	message(FATAL_ERROR "Platform not supported.")
endif()
string(REGEX REPLACE "\n" "" BUILD ${BUILD})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# This only works if building from the command line.  There is currently no way
# to set a variable's value based on the build type when using the MSVC IDE.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(BUILD "${BUILD}d")
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

#
# Build
#

set(DEFAULT_TVNC_BUILDSERVER 0)
if(UNIX AND NOT APPLE AND NOT CYGWIN)
	set(DEFAULT_TVNC_BUILDSERVER 1)
endif()
option(TVNC_BUILDSERVER "Build TurboVNC server components"
	${DEFAULT_TVNC_BUILDSERVER})

if(WIN32)
  add_subdirectory(win)
else()
  add_subdirectory(unix)
endif()
add_subdirectory(doc)
option(TVNC_BUILDJAVA "Build Java version of the TurboVNC Viewer" 0)
if(TVNC_BUILDJAVA)
	set(TVNC_BUILDJAVA 1)
	add_subdirectory(java)
else()
	set(TVNC_BUILDJAVA 0)
endif()


#
# Installation and packaging
#

include(cmakescripts/BuildPackages.cmake)

configure_file("${CMAKE_SOURCE_DIR}/cmakescripts/cmake_uninstall.cmake.in"
  "cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)
