# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_polymesh SHARED ${sources} ${headers_moc})

install(TARGETS psw_polymesh DESTINATION lib)

target_link_libraries(psw_polymesh ${LIBS} possumwood_sdk)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(polymesh SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(polymesh PROPERTIES PREFIX "")

install(TARGETS polymesh DESTINATION ${PLUGINS_INSTALL_PATH})
set_property(TARGET polymesh PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(polymesh psw_polymesh psw_render ${LIBS} possumwood_sdk GLEW)
