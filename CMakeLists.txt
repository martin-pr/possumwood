project (possumwood)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 2.8.4)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(LIBS "")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_CXX_STANDARD 14)

add_definitions(-Wall -Werror -g)

###########################################################
# DEPENDENCIES

# looking for boost
find_package(Boost REQUIRED COMPONENTS
	filesystem system
	serialization
	unit_test_framework
	program_options
	regex
	thread
)
include_directories(${Boost_INCLUDE_DIR})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# looking for OpenGL libraries
#find_package(OpenGL REQUIRED)
#include_directories(${OPENGL_INCLUDE_DIR})
#set(LIBS ${LIBS} ${OPENGL_LIBRARY})

# looking for glut
#find_package(GLUT REQUIRED)
#include_directories(${GLUT_INCLUDE_DIR})
#set(LIBS ${LIBS} ${GLUT_LIBRARY})

# blindly adding tbb
# set(LIBS ${LIBS} tbb)

###########################################################
# BUILD

include_directories(src/libs)
add_subdirectory(src)
