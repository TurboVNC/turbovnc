if(CMAKE_CL_64)
	set(WIN64DEF -DWIN64)
endif(CMAKE_CL_64)

set(CMAKE_RC_COMPILE_OBJECT
	"<CMAKE_RC_COMPILER> ${WIN64DEF} -D__RCVERSION=${RCVERSION} -D__RCVERSIONSTR=\\\"${RCVERSION}\\0\\\" -l 0x809 -fo <OBJECT> <SOURCE>")

add_library(VNCHooks SHARED
	VNCHooks.cpp
	VNCHooks.def
	VNCHooks.rc)

target_link_libraries(VNCHooks
	user32.lib gdi32.lib advapi32.lib)
