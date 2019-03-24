# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)

include_directories(./)

add_library(polymesh SHARED ${sources})
install(TARGETS polymesh DESTINATION ${PLUGINS_INSTALL_PATH})
set_property(TARGET polymesh PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(polymesh ${LIBS} render GLEW)
