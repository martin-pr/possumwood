set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

include_directories(./)

add_subdirectory(maths)
add_subdirectory(3d)
add_subdirectory(openmesh)
