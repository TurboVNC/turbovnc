include_directories(../../../xtrans)

add_definitions(-DUNIXCONN -DTCPCONN)
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	add_definitions(-DLOCALCONN)
elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# This is already defined for Linux
	add_definitions(-DHAVE_ABSTRACT_SOCKETS)
endif()

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

handle_type_puns(fstrans.c)
handle_unused(fstrans.c)

add_library(fc OBJECT
	fsconvert.c
	fserve.c
	fsio.c
	fstrans.c)
