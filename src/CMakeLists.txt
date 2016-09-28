include_directories(./)

file(GLOB headers_sandbox *.h)
file(GLOB sources_sandbox *.cpp)

set(headers ${headers_sandbox})
set(sources ${sources_sandbox})

QT4_WRAP_CPP(headers_moc ${headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(sandbox ${sources} ${headers_moc})

target_link_libraries(sandbox ${LIBS})
