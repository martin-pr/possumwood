# Find the source files
file(GLOB headers *.h)
file(GLOB_RECURSE sources *.cpp)
file(GLOB sources_common ../*.cpp)

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

QT5_WRAP_CPP(headers_moc ${headers})
add_definitions(${QT_DEFINITIONS})

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

# Find GLUT
FIND_PACKAGE(GLUT REQUIRED)
message(info ${GLUT_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GLUT_LIBRARY})

# Build the executable
include_directories(./)
qt5_add_resources(resources resources/resources.qrc)
add_executable(possumwood ${sources} ${sources_common} ${headers_moc} ${resources})
install(TARGETS possumwood DESTINATION bin)
set_property(TARGET possumwood PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(possumwood nodeeditor dependency_graph possumwood_sdk ${LIBS} dl GLEW)
