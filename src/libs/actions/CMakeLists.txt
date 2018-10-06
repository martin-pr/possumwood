include_directories(./)

# Building the library
file(GLOB_RECURSE sources *.cpp)

add_library(actions SHARED ${sources})

# Final linking
target_link_libraries(actions ${LIBS} GLEW GLU dependency_graph)
