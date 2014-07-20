include_directories(../../../extras/freetype2/include
	../../../extras/freetype2/src/truetype)

add_library(FreeType OBJECT
	ftfuncs.c
	ftenc.c
	fttools.c
	xttcap.c)
