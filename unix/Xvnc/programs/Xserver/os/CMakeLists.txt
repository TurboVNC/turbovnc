include_directories(../../../include ../../../include/extensions
	../../../lib/Xau ../../../lib/Xdmcp ../render)

add_definitions(-DXSERV_t -DTRANS_SERVER -DUNIXCONN -DTCPCONN -DHASXDMAUTH
	${ServerOSDefines} -DXDMCP ${EXT_DEFINES} -DUSE_RGB_TXT)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	add_definitions(-DBSD44SOCKETS -DNDBM)
endif()
set(RPCSRCS "")
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	add_definitions(-DNDBM -DSECURE_RPC)
	set(RPCSRCS rpcauth.c)
endif()

set(FFSL_SRC "")
if(NOT CMAKE_COMPILER_IS_GNUCC)
	set(FFSL_SRC ffsl.c)
endif()

add_library(os STATIC
	WaitFor.c
	access.c
	connection.c
	io.c
	oscolor.c
	osinit.c
	utils.c
	auth.c
	mitauth.c
	secauth.c
	xdmauth.c
	${RPCSRCS}
	xdmcp.c
	decompress.c
	../../../lib/xtrans/transport.c
	xalloc.c
	${FFSL_SRC})
