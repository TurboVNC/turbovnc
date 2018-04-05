include_directories(. ../Xi ../mi ../render)

handle_type_puns()

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
