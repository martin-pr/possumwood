# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

include_directories(./ ../cgal)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_anim SHARED ${sources} ${headers_moc})

install(TARGETS psw_anim DESTINATION lib)

target_link_libraries(psw_anim psw_cgal ${LIBS} possumwood_sdk)

#####

file(GLOB nodes_sources *.cpp datatypes/*.cpp ui/*.cpp)

add_library(anim SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(anim PROPERTIES PREFIX "")

install(TARGETS anim DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(anim psw_cgal psw_anim ${LIBS} possumwood_sdk)
