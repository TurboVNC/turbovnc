include_directories(../../extras/freetype2/include)

include(CheckFunctionExists)
check_function_exists(strerror HAVE_STRERROR)
if(HAVE_STRERROR)
	add_definitions(-DHAVE_STRERROR)
endif()

add_definitions(-DFT2_BUILD_LIBRARY)
if(APPLE)
	add_definitions(-DDARWIN_NO_CARBON)
endif()

set(FTSRC ../../extras/freetype2/src/)

add_library(freetype2 STATIC
	${FTSRC}/base/ftsystem.c
	${FTSRC}/base/ftinit.c
	${FTSRC}/base/ftdebug.c

	${FTSRC}/base/ftbase.c

	${FTSRC}/base/ftbbox.c
	${FTSRC}/base/ftglyph.c

	${FTSRC}/base/ftbdf.c
	${FTSRC}/base/ftbitmap.c
	${FTSRC}/base/ftcid.c
	${FTSRC}/base/ftfstype.c
	${FTSRC}/base/ftgasp.c
	${FTSRC}/base/ftgxval.c
	${FTSRC}/base/ftlcdfil.c
	${FTSRC}/base/ftmm.c
	${FTSRC}/base/ftotval.c
	${FTSRC}/base/ftpatent.c
	${FTSRC}/base/ftpfr.c
	${FTSRC}/base/ftstroke.c
	${FTSRC}/base/ftsynth.c
	${FTSRC}/base/fttype1.c
	${FTSRC}/base/ftwinfnt.c
	${FTSRC}/base/ftxf86.c

	${FTSRC}/bdf/bdf.c
	${FTSRC}/cff/cff.c
	${FTSRC}/cid/type1cid.c
	${FTSRC}/pcf/pcf.c
	${FTSRC}/pfr/pfr.c
	${FTSRC}/sfnt/sfnt.c

	${FTSRC}/truetype/truetype.c
	${FTSRC}/type1/type1.c
	${FTSRC}/type42/type42.c
	${FTSRC}/winfonts/winfnt.c

	${FTSRC}/raster/raster.c
	${FTSRC}/smooth/smooth.c

	${FTSRC}/autofit/autofit.c
	${FTSRC}/cache/ftcache.c
	${FTSRC}/gzip/ftgzip.c
	${FTSRC}/lzw/ftlzw.c
	${FTSRC}/bzip2/ftbzip2.c
	${FTSRC}/gxvalid/gxvalid.c
	${FTSRC}/otvalid/otvalid.c
	${FTSRC}/psaux/psaux.c
	${FTSRC}/pshinter/pshinter.c
	${FTSRC}/psnames/psnames.c)
