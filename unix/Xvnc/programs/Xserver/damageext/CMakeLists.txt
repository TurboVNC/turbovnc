include_directories(../Xi ../render ../xfixes)

handle_type_puns()

add_library(damageext STATIC
	damageext.c)
