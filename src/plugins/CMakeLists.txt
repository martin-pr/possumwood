set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)

include_directories(./)

add_subdirectory(maths)
add_subdirectory(anim)
add_subdirectory(render)
add_subdirectory(cgal)
add_subdirectory(polymesh)
add_subdirectory(images)
add_subdirectory(expressions)
