cmake_minimum_required(VERSION 2.6)

set(VERSION 1.0.80)
set(RCVERSION 1,0,80,0)

if(WIN32)
	execute_process(COMMAND "${CMAKE_SOURCE_DIR}/getdate.bat" OUTPUT_VARIABLE
		BUILD)
	string(REGEX REPLACE "\n" "" BUILD ${BUILD})
elseif(UNIX)
	execute_process(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE BUILD)
	string(REGEX REPLACE "\n" "" BUILD ${BUILD})
else(WIN32)
	message(FATAL_ERROR "date not implemented")
endif(WIN32)

# This ugly hack is necessary because the NMake generator always defaults to
# Debug.  Grrr...
if("${DEBUG}" EQUAL "1")
	set(CMAKE_BUILD_TYPE Debug)
else()
	set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type = ${CMAKE_BUILD_TYPE}")

foreach(var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
	if(${var} MATCHES "/MD")
		string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
	endif()
endforeach()

foreach(var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
	if(${var} MATCHES "/EHsc")
		string(REGEX REPLACE "/EHsc" "/EHs" ${var} "${${var}}")
	endif()
endforeach()

add_definitions(-wd4996 -wd4482 -D__WIN32__ -D_CRT_SECURE_NO_DEPRECATE
	-D__VERSION="${VERSION}" -D__BUILD="${BUILD}")

add_subdirectory(zlib)
add_subdirectory(vncviewer)
add_subdirectory(winvnc)
