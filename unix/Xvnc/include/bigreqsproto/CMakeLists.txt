set(HEADERS
	bigreqsproto.h
	bigreqstr.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
