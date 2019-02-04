# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# Find luabind
FIND_PACKAGE(PkgConfig REQUIRED)

pkg_check_modules (LUABIND luabind)

pkg_search_module(LUABIND REQUIRED)
include_directories(${LUABIND_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LUABIND_LIBRARIES})

######################################

file(GLOB_RECURSE expr_sources *.cpp)

include_directories(./)

add_library(lua SHARED ${expr_sources})

target_link_libraries(lua ${LIBS})
