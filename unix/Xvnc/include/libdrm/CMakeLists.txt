set(HEADERS
	include/drm/drm.h
	include/drm/drm_mode.h
	include/drm/drm_sarea.h)

foreach(h ${HEADERS})
	copy_X_header(${h} "")
endforeach()
