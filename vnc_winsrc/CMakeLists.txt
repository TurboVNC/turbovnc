cmake_minimum_required(VERSION 2.6)

project(TurboVNC)
set(VERSION 1.0.80)
set(RCVERSION 1,0,80,0)

if(NOT WIN32)
	message(FATAL_ERROR "TurboVNC cannot be cross-compiled")
endif()

if(NOT MSVC)
	message(FATAL_ERROR "TurboVNC requires Visual C++")
endif()

execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmakescripts/getdate.bat" OUTPUT_VARIABLE
	BUILD)
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

# Use the static C library for all build types
foreach(var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
	CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	if(${var} MATCHES "/MD")
		string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
	endif()
endforeach()

foreach(var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	if(${var} MATCHES "/EHsc")
		string(REGEX REPLACE "/EHsc" "/EHs" ${var} "${${var}}")
	endif()
endforeach()

# Don't auto-generate manifests
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

# Avoid linker warning when doing Debug build if dependent libraries are
# linked with the Release version of the static C library.
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")

add_definitions(-D__WIN32__ -U_MBCS -wd4996 -wd4482 -D_CRT_SECURE_NO_DEPRECATE)

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(INST_NAME ${CMAKE_PROJECT_NAME}64-${VERSION})
	set(64BIT 1)
	set(WIN64 1)
	set(WIN64DEF -DWIN64)
	message(STATUS "64-bit build")
else()
  set(INST_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
	message(STATUS "32-bit build")
endif()

include(cmakescripts/FindTurboJPEG.cmake)

if(MSVC_IDE)
	set(INSTALLERDIR "$(OutDir)")
	set(BUILDDIRDEF "-DBUILD_DIR=${INSTALLERDIR}\\")
else()
	set(INSTALLERDIR .)
	set(BUILDDIRDEF "-DBUILD_DIR=")
endif()

add_subdirectory(zlib)
add_subdirectory(vncviewer)
add_subdirectory(winvnc)
add_subdirectory(putty)
add_subdirectory(../vnc_docs doc)

configure_file(release/TurboVNC.iss.in pkgscripts/TurboVNC.iss)

add_custom_target(installer
	iscc -o${INSTALLERDIR} ${WIN64DEF} ${BUILDDIRDEF} -F${INST_NAME}
		pkgscripts/TurboVNC.iss
	DEPENDS vncviewer winvnc putty
	SOURCES pkgscripts/TurboVNC.iss)

install(FILES LICENCE.txt TurboVNC.url ../TurboVNC-ChangeLog.txt WhatsNew.txt
	DESTINATION .)
