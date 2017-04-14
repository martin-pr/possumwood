include_directories(./)

file(GLOB headers_lib *.h)
file(GLOB sources_lib *.cpp)

QT4_WRAP_CPP(headers_lib_moc ${headers_lib})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_library(nodeeditor SHARED ${sources_lib} ${headers_lib_moc})

target_link_libraries(nodeeditor ${LIBS})
