add_definitions(-DHAVE_PTHREAD)

disable_compiler_warnings()

check_function_exists(memfd_create HAVE_MEMFD_CREATE)
if(HAVE_MEMFD_CREATE)
	add_definitions(-DHAVE_MEMFD_CREATE)
endif()

check_symbol_exists(__NR_memfd_create asm/unistd.h HAVE_DECL___NR_MEMFD_CREATE)
if(HAVE_DECL___NR_MEMFD_CREATE)
	add_definitions(-DHAVE_DECL___NR_MEMFD_CREATE)
endif()

foreach(dir /run/shm /dev/shm /var/tmp /tmp)
	if(EXISTS ${dir})
		add_definitions(-DSHMDIR=\"${dir}\")
		break()
	endif()
endforeach()

add_library(xshmfence STATIC
	src/xshmfence_alloc.c
	src/xshmfence_pthread.c)
