include_directories(./)

# Building the library
file(GLOB_RECURSE sources *.cpp)

add_library(actions SHARED ${sources})
install(TARGETS actions DESTINATION lib)

# Final linking
target_link_libraries(actions ${LIBS} dependency_graph)
