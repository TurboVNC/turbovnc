include_directories(../mi ../render ../Xext)

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
