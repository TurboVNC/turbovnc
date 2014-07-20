set(HEADERS
	xf86bigfont.h
	xf86bigfproto.h
	xf86bigfstr.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
