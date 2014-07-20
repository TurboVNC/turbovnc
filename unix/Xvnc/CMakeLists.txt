# 2.8.8 introduced the "OBJECT" library target, which is used quite a bit in
# the Xvnc build.
cmake_minimum_required(VERSION 2.8.8)

include(CheckCSourceCompiles)

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

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
	message(STATUS "BSD-like system detected")
	add_definitions(-DCSRG_BASED)
elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
	NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	message(WARNING "TurboVNC Server build has not been tested on this platform.  Chaos may ensue.")
endif()

add_definitions(-DIPv6)

function(copy_X_header file dir)
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
	file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/unix/Xvnc/X_include/${dir})
endfunction()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/X_include
	${CMAKE_SOURCE_DIR}/common/zlib)

configure_file(include/tvnc_version.h.in
	programs/Xserver/include/tvnc_version.h)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(programs/Xserver)
