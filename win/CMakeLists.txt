enable_language(CXX)

if(BITS EQUAL 64)
	set(WIN64 1)
endif()

if(NOT MSVC)
	message(FATAL_ERROR "TurboVNC requires Visual C++")
endif()

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

add_subdirectory(${CMAKE_SOURCE_DIR}/common ${CMAKE_BINARY_DIR}/common)
add_subdirectory(vncviewer)
add_subdirectory(putty)

install(FILES ../LICENSE.txt TurboVNC.url ../ChangeLog.txt
	DESTINATION .)
