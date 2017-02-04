include_directories(./)

file(GLOB headers *.h)
file(GLOB sources *.cpp)

QT4_WRAP_CPP(headers_moc ${headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(possumwood ${sources} ${headers_moc})

target_link_libraries(possumwood nodeeditor dependency_graph ${LIBS})
