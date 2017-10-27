# Find CGAL
FIND_PACKAGE(CGAL REQUIRED)
set(LIBS ${LIBS} CGAL)

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)

include_directories(./)

add_library(cgal SHARED ${sources})

target_link_libraries(cgal maths ${LIBS})
