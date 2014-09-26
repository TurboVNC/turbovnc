add_definitions(-DUNIXCONN -DTCPCONN -DFONT_t -DTRANS_CLIENT -DTRANS_SERVER
	-DTRANS_REOPEN)

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

add_library(fc OBJECT
	fsconvert.c
	fserve.c
	fsio.c
	../../xtrans/transport.c)
