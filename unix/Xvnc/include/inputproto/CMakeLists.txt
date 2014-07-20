set(HEADERS
	XI.h
	XI2.h
	XI2proto.h
	XIproto.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
