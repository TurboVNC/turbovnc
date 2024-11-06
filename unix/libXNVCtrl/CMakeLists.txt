include(CheckIncludeFiles)

include_directories(${X11_INCLUDE_DIR})

if(NOT X11_Xext_FOUND)
	message(FATAL_ERROR "Could not find XEXT headers")
endif()

if(NOT X11_Xi_FOUND)
	message(FATAL_ERROR "Could not find Xi headers")
endif()

add_library(XNVCtrl STATIC NVCtrl.c)

target_link_libraries(XNVCtrl ${X11_LIBRARIES})
