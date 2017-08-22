include_directories(../Xi ../damageext ../mi  ../render ../xfixes)

handle_type_puns()

add_library(composite STATIC
	compalloc.c
	compext.c
	compinit.c
	compoverlay.c
	compwindow.c)
