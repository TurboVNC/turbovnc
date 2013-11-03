include_directories(../../../include ../../../include/extensions
	../../../lib/Xau ../../../lib/Xdmcp ../render)

add_definitions(-DXSERV_t -DTRANS_SERVER -DUNIXCONN -DTCPCONN -DHASXDMAUTH
	${ServerOSDefines} -DXDMCP ${EXT_DEFINES} -DUSE_RGB_TXT)

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
	xdmcp.c
	decompress.c
	../../../lib/xtrans/transport.c
	xalloc.c
	wrapper.c)
