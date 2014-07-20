set(HEADERS
	include/X11/Xlib.h
	include/X11/Xutil.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()
