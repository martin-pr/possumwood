include_directories(./)

file(GLOB sources *.cpp)

add_library(dependency_graph ${sources})

target_link_libraries(dependency_graph ${LIBS})
