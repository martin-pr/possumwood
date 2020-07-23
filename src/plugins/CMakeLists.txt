set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)

# all plugins compile into DSOs - they need -fPIC during compilation
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(./)

add_subdirectory(maths)
add_subdirectory(anim)
add_subdirectory(cgal)
add_subdirectory(lua)
add_subdirectory(render)
add_subdirectory(images)
add_subdirectory(opencv)
