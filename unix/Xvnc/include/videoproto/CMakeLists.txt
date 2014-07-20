set(HEADERS
	Xv.h
	XvMC.h
	XvMCproto.h
	Xvproto.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
