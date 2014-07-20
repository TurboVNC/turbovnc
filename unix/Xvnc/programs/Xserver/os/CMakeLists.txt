include_directories(../render ../../../lib/libsha1)

add_definitions(-DUNIXCONN -DTCPCONN -DIPV6CONN -DHASXDMAUTH ${ServerOSDefines}
	-DXDMCP -DHAVE_SHA1_IN_LIBSHA1)

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

set(RPCSRCS "")
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	add_definitions(-DSECURE_RPC)
	set(RPCSRCS rpcauth.c)
endif()

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

set(STRSRCS "")
if(NOT HAVE_STRCASECMP)
	set(STRSRCS ${STRSRCS} strcasecmp.c)
endif()
if(NOT HAVE_STRCASESTR)
	set(STRSRCS ${STRSRCS} strcasestr.c)
endif()
if(NOT HAVE_STRLCAT)
	set(STRSRCS ${STRSRCS} strlcat.c)
endif()
if(NOT HAVE_STRLCPY)
	set(STRSRCS ${STRSRCS} strlcpy.c)
endif()
if(NOT HAVE_STRNDUP)
	set(STRSRCS ${STRSRCS} strndup.c)
endif()

add_library(os STATIC
	WaitFor.c
	access.c
	auth.c
	backtrace.c
	client.c
	connection.c
	io.c
	log.c
	mitauth.c
	oscolor.c
	osinit.c
	rpcauth.c
	utils.c
	xdmauth.c
	xdmcp.c
	xprintf.c
	xsha1.c
	xstrans.c
	${RPCSRCS}
	${STRSRCS})
