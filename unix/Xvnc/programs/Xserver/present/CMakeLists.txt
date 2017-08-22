include_directories(../Xi ../miext/sync ../randr ../render ../xfixes)

add_definitions(-DHAVE_XORG_CONFIG_H)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(present STATIC
	present.c
	present_event.c
	present_fake.c
	present_fence.c
	present_notify.c
	present_request.c
	present_screen.c)
