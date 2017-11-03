include_directories(${FREETYPE_INCLUDE_DIRS})

add_library(FreeType OBJECT
	ftfuncs.c
	ftenc.c
	fttools.c
	xttcap.c)
