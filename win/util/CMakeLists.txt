add_definitions(-D__VERSION="${VERSION}" -D__BUILD="${BUILD}")

add_library(vglutil STATIC Thread.cpp bmp.c)

add_library(fbx STATIC fbx.c)

add_executable(fbxtest fbxtest.cpp)
target_link_libraries(fbxtest fbx vglutil)

add_executable(tcbench tcbench.cpp)
target_link_libraries(tcbench fbx)

install(TARGETS tcbench DESTINATION ${TVNC_BINDIR})
