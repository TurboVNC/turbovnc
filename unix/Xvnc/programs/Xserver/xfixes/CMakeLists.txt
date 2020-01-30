include_directories(. ../Xi ../mi ../render)

disable_compiler_warnings()

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
