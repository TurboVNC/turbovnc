project(TurboVNC)
cmake_minimum_required(VERSION 2.6)

set(VERSION 1.0.80)
set(RCVERSION 1,0,80,0)

if(NOT WIN32)
	message(FATAL_ERROR "TurboVNC cannot be cross-compiled")
endif()

if(NOT MSVC)
	message(FATAL_ERROR "TurboVNC requires Visual C++")
endif()

execute_process(COMMAND "${CMAKE_SOURCE_DIR}/getdate.bat" OUTPUT_VARIABLE
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

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")

add_definitions(-D__WIN32__ -D_CRT_SECURE_NO_DEPRECATE
	-D__VERSION="${VERSION}" -D__BUILD="${BUILD}")

add_definitions(-U_MBCS -wd4996 -wd4482 -D_CRT_SECURE_NO_DEPRECATE)

if(CMAKE_CL_64)
	set(WIN64 1)
	set(WIN64DEF -DWIN64)
endif()

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

configure_file(TurboVNC.iss.in TurboVNC.iss)

add_custom_target(installer
	iscc -o${INSTALLERDIR} ${WIN64DEF} ${BUILDDIRDEF} -FTurboVNC TurboVNC.iss
	DEPENDS vncviewer winvnc putty
	SOURCES TurboVNC.iss)
