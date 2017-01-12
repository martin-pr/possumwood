include_directories(./)

file(GLOB sources *.cpp)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(possumwood ${sources})

target_link_libraries(possumwood nodeeditor dependency_graph ${LIBS})
