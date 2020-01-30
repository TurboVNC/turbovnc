include_directories(include)

check_symbol_exists(strerror string.h HAVE_STRERROR)
if(HAVE_STRERROR)
	add_definitions(-DHAVE_STRERROR)
endif()

add_definitions(-DFT2_BUILD_LIBRARY)

disable_compiler_warnings()

add_library(freetype2 STATIC
	src/base/ftsystem.c
	src/base/ftinit.c
	src/base/ftdebug.c

	src/base/ftbase.c

	src/base/ftbbox.c
	src/base/ftglyph.c

	src/base/ftbdf.c
	src/base/ftbitmap.c
	src/base/ftcid.c
	src/base/ftfntfmt.c
	src/base/ftfstype.c
	src/base/ftgasp.c
	src/base/ftgxval.c
	src/base/ftlcdfil.c
	src/base/ftmm.c
	src/base/ftpsprop.c
	src/base/ftotval.c
	src/base/ftpatent.c
	src/base/ftpfr.c
	src/base/ftstroke.c
	src/base/ftsynth.c
	src/base/fttype1.c
	src/base/ftwinfnt.c

	src/bdf/bdf.c
	src/cff/cff.c
	src/cid/type1cid.c
	src/pcf/pcf.c
	src/pfr/pfr.c
	src/sfnt/sfnt.c
	src/truetype/truetype.c
	src/type1/type1.c
	src/type42/type42.c
	src/winfonts/winfnt.c

	src/raster/raster.c
	src/smooth/smooth.c

	src/autofit/autofit.c
	src/cache/ftcache.c
	src/gzip/ftgzip.c
	src/lzw/ftlzw.c
	src/bzip2/ftbzip2.c
	src/gxvalid/gxvalid.c
	src/otvalid/otvalid.c
	src/psaux/psaux.c
	src/pshinter/pshinter.c
	src/psnames/psnames.c)
