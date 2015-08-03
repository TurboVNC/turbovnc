add_library(XNVCtrl STATIC NVCtrl.c)

target_link_libraries(XNVCtrl X11 Xext)
