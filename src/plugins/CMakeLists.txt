set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

include_directories(./)

add_subdirectory(maths)
add_subdirectory(anim)
add_subdirectory(render)
add_subdirectory(cgal)
add_subdirectory(polymesh)