# Find CGAL
FIND_PACKAGE(CGAL REQUIRED)
set(LIBS ${LIBS} CGAL)

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)

# Find CGAL
find_package(CGAL)
set(LIBS ${LIBS} CGAL::CGAL)

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)

add_library(psw_cgal SHARED ${sources})

target_link_libraries(psw_cgal ${LIBS})


install(TARGETS psw_cgal DESTINATION lib)

#####

file(GLOB_RECURSE node_sources nodes/*.cpp)

add_library(cgal SHARED ${node_sources})
install(TARGETS cgal DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(cgal psw_cgal ${LIBS})
