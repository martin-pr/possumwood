# Find the source files
file(GLOB_RECURSE headers *.h)
file(GLOB_RECURSE sources *.cpp)
file(GLOB sources_common ../*.cpp)

# Find OpenGL
#FIND_PACKAGE(OpenGL REQUIRED)
#set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

# Find GLUT
#FIND_PACKAGE(GLUT REQUIRED)
#message(info ${GLUT_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${GLUT_LIBRARY})

# Build the executable
include_directories(./)
add_executable(possumwood_cli ${sources} ${sources_common})
install(TARGETS possumwood_cli DESTINATION bin)
set_property(TARGET possumwood_cli PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(possumwood_cli dependency_graph possumwood_sdk ${LIBS} dl GLEW GLU GL glfw OpenImageIO)
