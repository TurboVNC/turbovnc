add_definitions(-DBDFFORMAT -DPCFFORMAT -DSNFFORMAT -DX_GZIP_FONT_COMPRESSION)

add_library(bitmap OBJECT
	bdfread.c
	bdfutils.c
	bitmap.c
	bitmapfunc.c
	bitmaputil.c
	bitscale.c
	fontink.c
	pcfread.c
	pcfwrite.c
	snfread.c)
