set(HEADERS
	damageproto.h
	damagewire.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
