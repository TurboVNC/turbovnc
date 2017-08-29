include_directories(../mi ../render ../Xext)

handle_type_puns()

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
