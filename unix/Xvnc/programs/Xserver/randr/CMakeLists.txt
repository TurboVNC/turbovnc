include_directories(../mi ../render)

add_definitions(-DXINERAMA)

disable_compiler_warnings()
handle_type_puns()

add_library(randr STATIC
	randr.c
	rrcrtc.c
	rrdispatch.c
	rrinfo.c
	rrmode.c
	rrmonitor.c
	rroutput.c
	rrpointer.c
	rrproperty.c
	rrprovider.c
	rrproviderproperty.c
	rrscreen.c
	rrsdispatch.c
	rrtransform.c
	rrxinerama.c)
