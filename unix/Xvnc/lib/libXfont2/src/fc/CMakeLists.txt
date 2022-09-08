include_directories(../../../xtrans)

add_definitions(-DUNIXCONN -DTCPCONN)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# This is already defined for Linux
	add_definitions(-DHAVE_ABSTRACT_SOCKETS)
endif()

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

add_library(fc OBJECT
	fsconvert.c
	fserve.c
	fsio.c
	fstrans.c)
