set(HEADERS
	Xlib.h
	Xresource.h
	Xlibint.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()
