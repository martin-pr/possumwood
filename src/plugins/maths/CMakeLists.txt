file(GLOB_RECURSE sources *.cpp)

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

include_directories(./)

add_library(maths SHARED ${sources})

target_link_libraries(maths possumwood_sdk GL ${LIBS})
