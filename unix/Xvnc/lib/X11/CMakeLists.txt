set(HEADERS
	Xlib.h
	Xresource.h
	Xutil.h
	cursorfont.h
	Xlibint.h
	Xcms.h
	Xlocale.h
	XKBlib.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()
