# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)

include_directories(./)

add_library(render SHARED ${sources})
install(TARGETS render DESTINATION ${PLUGINS_INSTALL_PATH})
set_property(TARGET render PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)
set_property(TARGET render PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(render maths possumwood_cgal anim images ${LIBS} GLEW)
