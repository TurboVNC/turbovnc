set(HEADERS
	include/X11/extensions/XKBbells.h
	include/X11/extensions/XKBconfig.h
	include/X11/extensions/XKBfile.h
	include/X11/extensions/XKBrules.h
	include/X11/extensions/XKM.h
	include/X11/extensions/XKMformat.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
