include_directories(../Xi ../render ../xfixes)

handle_type_puns(damageext.c)

add_library(damageext STATIC
	damageext.c)
