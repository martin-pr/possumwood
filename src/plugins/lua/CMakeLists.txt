# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# Find luabind
FIND_PACKAGE(PkgConfig REQUIRED)

pkg_check_modules (LUABIND luabind)

pkg_search_module(LUABIND REQUIRED luabind)
include_directories(${LUABIND_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LUABIND_LIBRARIES})

message("************ ${LUABIND_LIBRARIES}")

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_lua SHARED ${sources} ${headers_moc})

install(TARGETS psw_lua DESTINATION lib)

target_link_libraries(psw_lua ${LIBS} possumwood_sdk)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(lua SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(lua PROPERTIES PREFIX "")

install(TARGETS lua DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(lua psw_lua ${LIBS} possumwood_sdk)

