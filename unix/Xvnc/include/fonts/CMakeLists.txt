set(HEADERS
	FS.h
	FSproto.h
	fsmasks.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/fonts)
endforeach()
