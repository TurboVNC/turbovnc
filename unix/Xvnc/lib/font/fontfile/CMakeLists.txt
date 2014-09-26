include_directories(../../../programs/Xserver/include)

add_definitions(-DFONTDIRATTRIB -DBUILD_SPEEDO -DBUILD_TYPE1 -DBUILD_CID
	-DBUILD_FREETYPE -DX_GZIP_FONT_COMPRESSION)

if(APPLE)
	set(DEFAULT_FONT_ENCODINGS_DIRECTORY "/opt/X11/share/fonts/encodings")
else()
	set(DEFAULT_FONT_ENCODINGS_DIRECTORY "/usr/share/X11/fonts/encodings")
endif()
set(FONT_ENCODINGS_DIRECTORY ${DEFAULT_FONT_ENCODINGS_DIRECTORY} CACHE PATH
	"Font encodings directory (default: ${DEFAULT_FONT_ENCODINGS_DIRECTORY})")
message(STATUS "FONT_ENCODINGS_DIRECTORY = ${FONT_ENCODINGS_DIRECTORY}")
add_definitions(-DFONT_ENCODINGS_DIRECTORY=\"${FONT_ENCODINGS_DIRECTORY}\")

add_library(fontfile OBJECT
	catalogue.c
	dirfile.c
	fontdir.c
	fontfile.c
	fileio.c
	fontscale.c
	defaults.c
	bitsource.c
	register.c
	renderers.c
	bufio.c
	decompress.c
	filewr.c
	printerfont.c
	ffcheck.c
	fontenc.c
	encparse.c
	gunzip.c)
