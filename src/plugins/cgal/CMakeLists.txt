# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
add_definitions(-Wno-error=ignored-optimization-argument) # not supported by clang
endif()

# Find CGAL
find_package(CGAL REQUIRED)
set(LIBS ${LIBS} CGAL::CGAL)

# Find luabind
FIND_PACKAGE(PkgConfig REQUIRED)

pkg_check_modules (LUABIND luabind)

pkg_search_module(LUABIND REQUIRED)
include_directories(${LUABIND_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LUABIND_LIBRARIES})

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)

add_library(psw_cgal SHARED ${sources})

target_link_libraries(psw_cgal ${LIBS} dependency_graph possumwood_sdk)


install(TARGETS psw_cgal DESTINATION lib)

#####

file(GLOB_RECURSE node_sources nodes/*.cpp lua/*.cpp)

add_library(cgal SHARED ${node_sources})
install(TARGETS cgal DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(cgal psw_cgal psw_lua ${LIBS})
