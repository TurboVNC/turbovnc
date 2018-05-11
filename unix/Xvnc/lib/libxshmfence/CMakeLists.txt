add_definitions(-DHAVE_PTHREAD)

disable_compiler_warnings()

foreach(dir /run/shm /dev/shm /var/tmp /tmp)
	if(EXISTS ${dir})
		add_definitions(-DSHMDIR=\"${dir}\")
		break()
	endif()
endforeach()

add_library(xshmfence STATIC
	src/xshmfence_alloc.c
	src/xshmfence_pthread.c)
