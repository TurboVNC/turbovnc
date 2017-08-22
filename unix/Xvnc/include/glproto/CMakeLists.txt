set(HEADERS
	glxint.h
	glxmd.h
	glxproto.h
	glxtokens.h)

foreach(h ${HEADERS})
	copy_X_header(${h} GL)
endforeach()
