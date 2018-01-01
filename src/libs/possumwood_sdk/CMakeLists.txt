include_directories(./)

# Building the library
file(GLOB_RECURSE headers *.h)
file(GLOB_RECURSE sources *.cpp)

# Qt fun
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
set(LIBS ${LIBS} Qt5::Widgets)

QT5_WRAP_CPP(headers_moc ${headers})
add_definitions(${QT_DEFINITIONS})
add_library(possumwood_sdk SHARED ${sources} ${headers_moc})

# Final linking
target_link_libraries(possumwood_sdk ${LIBS} GLEW)
