include_directories(./)

# Building the library
file(GLOB headers_lib *.h)
file(GLOB sources_lib *.cpp)

# Qt fun
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
set(LIBS ${LIBS} Qt5::Widgets)

QT5_WRAP_CPP(headers_lib_moc ${headers_lib})
add_definitions(${QT_DEFINITIONS})
add_library(nodeeditor SHARED ${sources_lib} ${headers_lib_moc})

# Final linking
target_link_libraries(nodeeditor ${LIBS})
