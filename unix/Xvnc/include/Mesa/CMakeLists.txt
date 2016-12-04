set(HEADERS
	include/GL/gl.h
	include/GL/glext.h)

foreach(h ${HEADERS})
	copy_X_header(${h} GL)
endforeach()

copy_x_header(include/GL/internal/dri_interface.h GL/internal)
