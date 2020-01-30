include_directories(../Xi ../render ../xfixes)

disable_compiler_warnings()

add_library(damageext STATIC
	damageext.c)
