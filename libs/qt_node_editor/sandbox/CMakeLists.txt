include_directories(./)

file(GLOB sources_sandbox *.cpp)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(nodeeditor_sandbox ${sources_sandbox})

target_link_libraries(nodeeditor_sandbox nodeeditor ${LIBS})
