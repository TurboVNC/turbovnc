include_directories(. ${CMAKE_SOURCE_DIR}/common/d3des)

add_library(vncauth STATIC
	vncauth.c
	${CMAKE_SOURCE_DIR}/common/d3des/d3des.c
	${CMAKE_SOURCE_DIR}/common/d3des/d3des.h)
