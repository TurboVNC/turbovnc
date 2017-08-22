include_directories(../damageext ../mi ../miext/damage ../render ../xfixes)

handle_type_puns()

add_library(composite STATIC
	compalloc.c
	compext.c
	compinit.c
	compoverlay.c
	compwindow.c)
