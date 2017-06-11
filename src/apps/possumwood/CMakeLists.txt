# Find the source files
file(GLOB headers *.h)
file(GLOB_RECURSE sources *.cpp)

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
add_executable(possumwood ${sources} ${headers_moc})

target_link_libraries(possumwood nodeeditor dependency_graph possumwood_sdk ${LIBS} dl)
