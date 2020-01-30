set(HEADERS
	src/glx.h
	src/xcb.h
	src/xcbext.h
	src/xproto.h)

foreach(h ${HEADERS})
	copy_X_header(${h} xcb)
endforeach()

check_symbol_exists(sendmsg sys/socket.h HAVE_SENDMSG)
if(HAVE_SENDMSG)
	add_definitions(-DHAVE_SENDMSG)
endif()
add_definitions(-DXCB_QUEUE_BUFFER_SIZE=16384)

disable_compiler_warnings()

add_library(xcbglx STATIC
	src/bigreq.c
	src/glx.c
	src/xc_misc.c
	src/xcb_conn.c
	src/xcb_ext.c
	src/xcb_in.c
	src/xcb_list.c
	src/xcb_out.c
	src/xcb_xid.c
	src/xproto.c)
