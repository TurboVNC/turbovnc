include_directories(${X11_INCLUDE_DIR})

add_library(XNVCtrl STATIC NVCtrl.c)

target_link_libraries(XNVCtrl ${X11_LIBRARIES})
