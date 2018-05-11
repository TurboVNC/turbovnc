include_directories(../Xi ../damageext ../mi  ../render ../xfixes)

disable_compiler_warnings()
handle_type_puns(compext.c)

add_library(composite STATIC
	compalloc.c
	compext.c
	compinit.c
	compoverlay.c
	compwindow.c)
