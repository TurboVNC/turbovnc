set(HEADERS
	XKB.h
	XKBgeom.h
	XKBproto.h
	XKBsrv.h
	XKBstr.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
