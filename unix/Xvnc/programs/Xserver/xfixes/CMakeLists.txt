include_directories(. ../Xi ../mi ../render)

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
