set(HEADERS
	FS.h
	FSproto.h
	font.h
	fontproto.h
	fontstruct.h
	fsmasks.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/fonts)
endforeach()
