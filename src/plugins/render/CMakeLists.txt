# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)

include_directories(./)

add_library(render SHARED ${sources})

target_link_libraries(render maths cgal anim images ${LIBS} GLEW)
