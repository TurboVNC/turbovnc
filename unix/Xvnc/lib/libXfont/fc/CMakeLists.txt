add_definitions(-DUNIXCONN -DTCPCONN -DIPV6CONN)

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

add_library(fc OBJECT
	fsconvert.c
	fserve.c
	fsio.c
	fstrans.c)
