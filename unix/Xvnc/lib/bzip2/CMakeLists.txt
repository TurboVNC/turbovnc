disable_compiler_warnings()

add_library(bzip2 STATIC
	blocksort.c
	bzlib.c
	compress.c
	crctable.c
	decompress.c
	huffman.c
	randtable.c)
