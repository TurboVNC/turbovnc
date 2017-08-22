set(HEADERS
	include/X11/extensions/Xext.h
	include/X11/extensions/extutil.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
