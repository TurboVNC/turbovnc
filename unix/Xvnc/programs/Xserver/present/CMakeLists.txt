include_directories(../Xi ../miext/sync ../randr ../render ../xfixes)

add_definitions(-DHAVE_XORG_CONFIG_H)

handle_type_puns()

add_library(present STATIC
	present.c
	present_event.c
	present_fake.c
	present_fence.c
	present_notify.c
	present_request.c
	present_screen.c)
