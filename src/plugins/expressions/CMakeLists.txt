# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE expr_sources *.cpp)

include_directories(./)

add_library(expressions SHARED ${expr_sources})

target_link_libraries(expressions ${LIBS})
