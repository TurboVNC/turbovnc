include_directories(../render ../../../lib/libsha1)

add_definitions(-DUNIXCONN -DTCPCONN -DHASXDMAUTH ${ServerOSDefines} -DXDMCP
	-DHAVE_SHA1_IN_LIBSHA1)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# This is already defined for Linux
	add_definitions(-DHAVE_ABSTRACT_SOCKETS)
endif()

if(FOUND_BSD44SOCKETS)
	add_definitions(-DBSD44SOCKETS)
endif()

set(EXTRASRCS "")
if(NOT HAVE_REALLOCARRAY)
	set(EXTRASRCS ${EXTRASRCS} reallocarray.c)
endif()
if(NOT HAVE_STRCASECMP)
	set(EXTRASRCS ${EXTRASRCS} strcasecmp.c)
endif()
if(NOT HAVE_STRCASESTR)
	set(EXTRASRCS ${EXTRASRCS} strcasestr.c)
endif()
if(NOT HAVE_STRLCAT)
	set(EXTRASRCS ${EXTRASRCS} strlcat.c)
endif()
if(NOT HAVE_STRLCPY)
	set(EXTRASRCS ${EXTRASRCS} strlcpy.c)
endif()
if(NOT HAVE_STRNDUP)
	set(EXTRASRCS ${EXTRASRCS} strndup.c)
endif()
if(NOT HAVE_TIMINGSAFE_MEMCMP)
	set(EXTRASRCS ${EXTRASRCS} timingsafe_memcmp.c)
endif()
if(NOT HAVE_EPOLL_CREATE1 AND NOT HAVE_POLL)
	set(EXTRASRCS ${EXTRASRCS} xserver_poll.c)
endif()

disable_compiler_warnings()

add_library(os STATIC
	WaitFor.c
	access.c
	auth.c
	backtrace.c
	busfault.c
	client.c
	connection.c
	inputthread.c
	io.c
	log.c
	mitauth.c
	oscolor.c
	osinit.c
	ospoll.c
	reallocarray.c
	rpcauth.c
	utils.c
	xdmauth.c
	xdmcp.c
	xprintf.c
	xsha1.c
	xstrans.c
	${EXTRASRCS})
