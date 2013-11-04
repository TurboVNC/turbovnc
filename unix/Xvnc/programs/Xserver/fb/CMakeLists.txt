include_directories(../mi ../render)

add_definitions(-DFB_OLD_GC -DFB_OLD_SCREEN)
if((CPU_TYPE STREQUAL "x86_64" OR CPU_TYPE STREQUAL "i386") AND GCC34)
	add_definitions(-mmmx -msse -Winline --param inline-unit-growth=10000
		--param large-function-growth=10000 -DUSE_MMX)
endif()

add_library(fb STATIC
	fballpriv.c
	fbarc.c
	fbbits.c
	fbblt.c
	fbbltone.c
	fbbstore.c
	fbcmap.c
	fbcompose.c
	fbcopy.c
	fbedge.c
	fbfill.c
	fbfillrect.c
	fbfillsp.c
	fbgc.c
	fbgetsp.c
	fbglyph.c
	fbimage.c
	fbline.c
	fboverlay.c
	fbpixmap.c
	fbpoint.c
	fbpush.c
	fbscreen.c
	fbseg.c
	fbsetsp.c
	fbsolid.c
	fbstipple.c
	fbtile.c
	fbtrap.c
	fbutil.c
	fbwindow.c
	fb24_32.c
	fbpict.c
	fbmmx.c)
