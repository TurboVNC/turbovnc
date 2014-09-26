set(HEADERS
	X.h
	Xalloca.h
	Xatom.h
	Xdefs.h
	Xfuncproto.h
	Xfuncs.h
	Xmd.h
	Xos.h
	Xos_r.h
	Xosdefs.h
	Xpoll.h
	Xproto.h
	Xprotostr.h
	Xthreads.h
	keysym.h
	keysymdef.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()

add_subdirectory(fonts)
add_subdirectory(extensions)
