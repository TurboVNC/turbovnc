include_directories(../Xi ../miext/sync ../randr ../render ../xfixes)

add_definitions(-DHAVE_XORG_CONFIG_H)

disable_compiler_warnings()
handle_type_puns(
	present_event.c
	present_request.c)

add_library(present STATIC
	present.c
	present_event.c
	present_fake.c
	present_fence.c
	present_notify.c
	present_request.c
	present_screen.c)
