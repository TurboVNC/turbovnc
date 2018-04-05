include_directories(${BZIP2_INCLUDE_DIR})

add_library(fontfile OBJECT
	bitsource.c
	bufio.c
	bunzip2.c
	catalogue.c
	decompress.c
	defaults.c
	dirfile.c
	fileio.c
	filewr.c
	fontdir.c
	fontencc.c
	fontfile.c
	fontscale.c
	gunzip.c
	register.c
	renderers.c)
