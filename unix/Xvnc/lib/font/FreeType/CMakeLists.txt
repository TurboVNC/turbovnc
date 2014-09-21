include_directories(../../../extras/freetype2/include
	../../../extras/freetype2/src/truetype)

add_definitions(${ServerExtraDefines} -DXFREE86_FT2)

add_library(FreeType OBJECT
	xttcap.c
	ftfuncs.c
	ftenc.c
	fttools.c)
