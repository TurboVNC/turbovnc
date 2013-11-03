# 2.8.8 introduced the "OBJECT" library target, which is used quite a bit in
# the Xvnc build.
cmake_minimum_required(VERSION 2.8.8)

if(CPU_TYPE STREQUAL "x86_64")
	add_definitions(-D__x86_64__ -D__AMD64__)
elseif(CPU_TYPE STREQUAL "i386")
	add_definitions(-D__i386__)
elseif(CPU_TYPE STREQUAL "arm")
	add_definitions(-D__arm__ -D__arm32__)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_definitions(-Dlinux)
	add_definitions(-D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE
		-D_XOPEN_SOURCE=500L -D_BSD_SOURCE -D_SVID_SOURCE)
endif()

add_definitions(-D_REENTRANT -DFUNCPROTO=15 -DNARROWPROTO)

set(ServerExtraDefines -DSTATIC_COLOR -DAVOID_GLYPHBLT -DPIXPRIV)
if(CMAKE_COMPILER_IS_GNUCC)
	set(ServerExtraDefines -DGCCUSESGAS ${ServerExtraDefines})
endif()
if(BITS EQUAL 64)
	set(ServerExtraDefines ${ServerExtraDefines} -D_XSERVER64)
endif()

function(copy_X_header file dir)
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
	file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
endfunction()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/X_include
	${CMAKE_CURRENT_BINARY_DIR}/X_include/X11)

configure_file(include/tvnc_version.h.in include/tvnc_version.h)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(programs/Xserver)
