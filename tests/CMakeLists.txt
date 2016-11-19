include_directories(./)

file(GLOB sources *.cpp)

add_executable(dependency_graph_tests ${sources})

target_link_libraries(dependency_graph_tests ${LIBS} dependency_graph)
