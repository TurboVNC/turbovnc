set(USE_FDS_BITS fds_bits)

configure_file(Xfuncproto.h.in Xfuncproto.h)
configure_file(Xpoll.h.in Xpoll.h)

set(HEADERS
	X.h
	XWDFile.h
	Xalloca.h
	Xarch.h
	Xatom.h
	Xdefs.h
	${CMAKE_CURRENT_BINARY_DIR}/Xfuncproto.h
	Xfuncs.h
	Xmd.h
	Xos.h
	Xos_r.h
	Xosdefs.h
	${CMAKE_CURRENT_BINARY_DIR}/Xpoll.h
	Xproto.h
	Xprotostr.h
	Xthreads.h
	keysym.h
	keysymdef.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()
