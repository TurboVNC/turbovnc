include_directories(../Xi ../miext/sync ../randr ../render ../xfixes)

add_definitions(-DHAVE_XORG_CONFIG_H)

disable_compiler_warnings()

add_library(present STATIC
	present.c
	present_event.c
	present_execute.c
	present_fake.c
	present_fence.c
	present_notify.c
	present_request.c
	present_scmd.c
	present_screen.c
	present_vblank.c
	present_wnmd.c)
