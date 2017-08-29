include_directories(../miext/damage ../xfixes)

handle_type_puns()

add_library(damageext STATIC
	damageext.c)
