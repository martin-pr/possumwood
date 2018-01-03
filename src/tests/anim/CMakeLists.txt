include_directories(./)
include_directories(../../plugins)

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

file(GLOB sources *.cpp)

message(${LIBS})

add_executable(anim_tests ${sources})

target_link_libraries(anim_tests ${LIBS} anim maths dependency_graph)
