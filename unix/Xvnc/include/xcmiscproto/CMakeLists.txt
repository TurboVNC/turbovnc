set(HEADERS
	xcmiscproto.h
	xcmiscstr.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
