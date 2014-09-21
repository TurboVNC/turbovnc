include_directories(../../extras/freetype2/include)

# This code has some type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_definitions(-DXTHREADS -DXUSE_MTSAFE_API -DXFREE86_FT2)
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	add_definitions(-DXUSE_NETDB_R_API)
endif()
if(APPLE)
	add_definitions(-DDARWIN_NO_CARBON)
endif()

set(FTSRC ../../extras/freetype2/src/)

add_library(freetype2 STATIC
	${FTSRC}/base/ftapi.c
	${FTSRC}/base/ftbase.c
	${FTSRC}/base/ftbbox.c
	${FTSRC}/base/ftbdf.c
	${FTSRC}/base/ftdebug.c
	${FTSRC}/base/ftglyph.c
	${FTSRC}/base/ftinit.c
	${FTSRC}/base/ftmm.c
	${FTSRC}/base/ftpfr.c
	${FTSRC}/base/ftstroker.c
	${FTSRC}/base/ftsynth.c
	${FTSRC}/base/ftsystem.c
	${FTSRC}/base/fttype1.c
	${FTSRC}/base/ftwinfnt.c
	${FTSRC}/base/ftxf86.c
	${FTSRC}/autohint/autohint.c
	${FTSRC}/bdf/bdf.c
	${FTSRC}/cff/cff.c
	${FTSRC}/cid/type1cid.c
	${FTSRC}/gzip/ftgzip.c
	${FTSRC}/pcf/pcf.c
	${FTSRC}/pfr/pfr.c
	${FTSRC}/psaux/psaux.c
	${FTSRC}/pshinter/pshinter.c
	${FTSRC}/psnames/psnames.c
	${FTSRC}/raster/raster.c
	${FTSRC}/sfnt/sfnt.c
	${FTSRC}/smooth/smooth.c
	${FTSRC}/truetype/truetype.c
	${FTSRC}/type1/type1.c
	${FTSRC}/type42/type42.c
	${FTSRC}/winfonts/winfnt.c)
