###########################################################
# BUILDING THE SDK

include_directories(./)

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

# Building the library
file(GLOB_RECURSE headers *.h)
file(GLOB_RECURSE sources *.cpp)

# Qt fun
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
set(LIBS ${LIBS} Qt5::Widgets)

QT5_WRAP_CPP(headers_moc ${headers})
add_definitions(${QT_DEFINITIONS})

# Qt 5 on Ubuntu 20.10 generates a lot of deprecation warnings,
# which are turned into errors in our build. Let's disable them
# for now.
add_definitions(-Wno-error=deprecated-declarations)

add_library(possumwood_sdk SHARED ${sources})
install(TARGETS possumwood_sdk DESTINATION lib)

# Final linking
target_link_libraries(possumwood_sdk ${LIBS} GLU GLEW dependency_graph actions)
