if(NOT HAVE_STRLCAT)
	set(EXTRASRCS ${EXTRASRCS} strlcat.c)
endif()
if(NOT HAVE_STRLCPY)
	set(EXTRASRCS ${EXTRASRCS} strlcpy.c)
endif()

add_library(util OBJECT
	fontaccel.c
	fontnames.c
	fontutil.c
	fontxlfd.c
	format.c
	miscutil.c
	patcache.c
	private.c
	reallocarray.c
	utilbitmap.c
	${EXTRASRCS})
