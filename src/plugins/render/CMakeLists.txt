# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_render SHARED ${sources} ${headers_moc})

install(TARGETS psw_render DESTINATION lib)

target_link_libraries(psw_render psw_images psw_cgal psw_anim ${LIBS} possumwood_sdk)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(render SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(render PROPERTIES PREFIX "")

install(TARGETS render DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(render psw_render ${LIBS} possumwood_sdk GLEW)
