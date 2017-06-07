file(GLOB_RECURSE sources *.cpp)

add_library(maths SHARED ${sources})

target_link_libraries(maths ${LIBS})
