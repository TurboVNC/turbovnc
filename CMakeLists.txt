###############################################################################
# SETUP
###############################################################################

cmake_minimum_required(VERSION 3.16...3.28)

project(TurboVNC NONE)
string(TOLOWER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_LC)
set(VERSION 3.2.1)
set(DOCVERSION 3.2.1)
set(COPYRIGHT_YEAR "1999-2025")
set(COPYRIGHT "The VirtualGL Project and many others (see README.md)")
set(URLTEXT "Visit http://www.TurboVNC.org for more information on TurboVNC")

macro(boolean_number var)
	if(${var})
		set(${var} 1 ${ARGN})
	else()
		set(${var} 0 ${ARGN})
	endif()
endmacro()

macro(report_option var desc)
	if(${var})
		message(STATUS "${desc} enabled (${var} = ${${var}})")
	else()
		message(STATUS "${desc} disabled (${var} = ${${var}})")
	endif()
endmacro()

set(DEFAULT_TVNC_BUILDVIEWER 1)
option(TVNC_BUILDVIEWER "Build TurboVNC Viewer"
	${DEFAULT_TVNC_BUILDVIEWER})
boolean_number(TVNC_BUILDVIEWER)
report_option(TVNC_BUILDVIEWER "TurboVNC Viewer")

set(DEFAULT_TVNC_BUILDSERVER 0)
if(UNIX AND NOT APPLE AND NOT CYGWIN AND
	NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(DEFAULT_TVNC_BUILDSERVER 1)
endif()
option(TVNC_BUILDSERVER "Build TurboVNC server components"
	${DEFAULT_TVNC_BUILDSERVER})
boolean_number(TVNC_BUILDSERVER)
report_option(TVNC_BUILDSERVER "TurboVNC Server")

if(NOT TVNC_BUILDVIEWER AND NOT TVNC_BUILDSERVER)
	message(FATAL_ERROR "Either TVNC_BUILDVIEWER or TVNC_BUILDSERVER must be enabled.")
endif()

enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

function(disable_compiler_warnings)
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w" PARENT_SCOPE)
	endif()
	if(CMAKE_C_COMPILER_ID MATCHES SunPro)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -erroff" PARENT_SCOPE)
	endif()
endfunction()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

string(TIMESTAMP DEFAULT_BUILD "%Y%m%d")
set(BUILD ${DEFAULT_BUILD} CACHE STRING "Build string (default: ${DEFAULT_BUILD})")

message(STATUS "VERSION = ${VERSION}, BUILD = ${BUILD}")

if(UNIX)
	set(CMAKE_INSTALL_DEFAULT_PREFIX /opt/${CMAKE_PROJECT_NAME})
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_DEFAULT_PREFIX}" CACHE PATH
			"Directory into which to install ${CMAKE_PROJECT_NAME} (default: ${CMAKE_INSTALL_DEFAULT_PREFIX})"
			FORCE)
	endif()
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

# When the prefix is /opt/${CMAKE_PROJECT_NAME}, we assume that an "official"
# build is being created, and thus we install things into specific locations.

if((UNIX AND CMAKE_INSTALL_PREFIX STREQUAL "${CMAKE_INSTALL_DEFAULT_PREFIX}")
	OR WIN32)
	set(CMAKE_INSTALL_DEFAULT_DATAROOTDIR "")
	set(CMAKE_INSTALL_DEFAULT_DOCDIR "<CMAKE_INSTALL_DATAROOTDIR>/doc")
	set(CMAKE_INSTALL_DEFAULT_SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc")
endif()
if(WIN32)
	set(CMAKE_INSTALL_DEFAULT_BINDIR ".")
endif()

include(cmakescripts/GNUInstallDirs.cmake)

macro(report_directory var)
	if(CMAKE_INSTALL_${var} STREQUAL CMAKE_INSTALL_FULL_${var})
		message(STATUS "CMAKE_INSTALL_${var} = ${CMAKE_INSTALL_${var}}")
	else()
		message(STATUS "CMAKE_INSTALL_${var} = ${CMAKE_INSTALL_${var}} (${CMAKE_INSTALL_FULL_${var}})")
	endif()
	mark_as_advanced(CLEAR CMAKE_INSTALL_${var})
endmacro()

set(DIRLIST "BINDIR;DATAROOTDIR;DATADIR;DOCDIR")
if(UNIX)
	list(APPEND DIRLIST "MANDIR")
	if(TVNC_BUILDSERVER)
		list(APPEND DIRLIST "SYSCONFDIR")
	endif()
endif()
foreach(dir ${DIRLIST})
	report_directory(${dir})
endforeach()

# Detect CPU type and word size
math(EXPR BITS "${CMAKE_SIZEOF_VOID_P} * 8")
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} CMAKE_SYSTEM_PROCESSOR_LC)
set(COUNT 1)
foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
	if(COUNT GREATER 1)
		message(FATAL_ERROR "TurboVNC contains JNI code, so it cannot be built with multiple values in CMAKE_OSX_ARCHITECTURES.")
	endif()
	math(EXPR COUNT "${COUNT}+1")
endforeach()
if(CMAKE_SYSTEM_PROCESSOR_LC MATCHES "x86_64" OR
	CMAKE_SYSTEM_PROCESSOR_LC MATCHES "amd64" OR
	CMAKE_SYSTEM_PROCESSOR_LC MATCHES "i[0-9]86" OR
	CMAKE_SYSTEM_PROCESSOR_LC MATCHES "x86" OR
	CMAKE_SYSTEM_PROCESSOR_LC MATCHES "ia32")
	if(BITS EQUAL 64)
		set(CPU_TYPE x86_64)
	else()
		set(CPU_TYPE i386)
	endif()
	if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL ${CPU_TYPE})
		set(CMAKE_SYSTEM_PROCESSOR ${CPU_TYPE})
	endif()
elseif(CMAKE_SYSTEM_PROCESSOR_LC STREQUAL "aarch64" OR
	CMAKE_SYSTEM_PROCESSOR_LC MATCHES "^arm")
	if(BITS EQUAL 64)
		set(CPU_TYPE arm64)
	else()
		set(CPU_TYPE arm)
	endif()
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" AND BITS EQUAL 32)
	set(CPU_TYPE ppc)
	set(CMAKE_SYSTEM_PROCESSOR ${CPU_TYPE})
else()
	set(CPU_TYPE ${CMAKE_SYSTEM_PROCESSOR_LC})
endif()
if(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" OR
	CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR
	CMAKE_OSX_ARCHITECTURES MATCHES "i386" OR
	CMAKE_OSX_ARCHITECTURES MATCHES "ppc")
	set(CPU_TYPE ${CMAKE_OSX_ARCHITECTURES})
endif()
message(STATUS "${BITS}-bit build (${CPU_TYPE})")
include(cmakescripts/FindTurboJPEG.cmake)


###############################################################################
# BUILD
###############################################################################

if(TVNC_BUILDVIEWER)
	add_subdirectory(java)
endif()
if(WIN32 AND TVNC_BUILDVIEWER)
	add_subdirectory(win)
endif()
if(UNIX)
	add_subdirectory(unix)
endif()
add_subdirectory(doc)


###############################################################################
# INSTALLATION AND PACKAGING
###############################################################################

include(cmakescripts/BuildPackages.cmake)

configure_file("${CMAKE_SOURCE_DIR}/cmakescripts/cmake_uninstall.cmake.in"
	"cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)

if(CMAKE_INSTALL_PREFIX STREQUAL "${CMAKE_INSTALL_DEFAULT_PREFIX}" OR WIN32)
	install(FILES README.md DESTINATION .)
else()
	install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
