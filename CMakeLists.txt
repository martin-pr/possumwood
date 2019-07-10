project (possumwood CXX)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 3.0.0)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# this is an anti-pattern, but CMAKE does seem to be picking up CMAKE_CXX_FLAGS_RELEASE and CMAKE_CXX_FLAGS_DEBUG
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function")
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -g")
else()
	message(ERROR "Unknown build type.")
endif()

set(CMAKE_CXX_STANDARD 14)

add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)
add_definitions(-Wl,--no-undefined)

# required by CGAL
add_definitions(-DBOOST_PARAMETER_MAX_ARITY=12)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# DNEG-specific hack, should have no impact for anything else
include_directories($ENV{BOB_WORLD_SLOT_ilmbase}/include/OpenEXR)

###########################################################
# DEPENDENCIES

set(LIBS "")

# looking for pthreads + friends
find_package (Threads REQUIRED)
set(LIBS ${LIBS} Threads::Threads)

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

# looking for openexr libs
SET(OPENEXR_LIBRARY_PATHS /usr/lib /usr/local/lib /sw/lib /opt/local/lib $ENV{PROGRAM_FILES}/OpenEXR/lib/static)
FIND_PATH(OPENEXR_INCLUDE_DIR ImfRgbaFile.h PATH_SUFFIXES OpenEXR /usr/include /usr/local/include /sw/include /opt/local/include)
FIND_LIBRARY(OPENEXR_HALF_LIBRARY NAMES Half PATHS ${OPENEXR_LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_IEX_LIBRARY NAMES Iex PATHS ${OPENEXR_LIBRARY_PATHS})
FIND_LIBRARY(OPENEXR_ILMIMF_LIBRARY NAMES IlmImf PATHS ${OPENEXR_LIBRARY_PATHS})
if (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARY AND OPENEXR_ILMIMF_LIBRARY)
  set (HDF5_FOUND TRUE)
  message (STATUS "OpenEXR library found (${OPENEXR_INCLUDE_DIR}, ${OPENEXR_IEX_LIBRARY} and ${OPENEXR_HALF_LIBRARY} and ${OPENEXR_ILMIMF_LIBRARY}).")
else (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARYAND OPENEXR_ILMIMF_LIBRARY)
  message (FATAL_ERROR "OpenEXR library not found (${OPENEXR_INCLUDE_DIR}, ${OPENEXR_IEX_LIBRARY} and ${OPENEXR_HALF_LIBRARY} and ${OPENEXR_ILMIMF_LIBRARY}).")
endif (OPENEXR_INCLUDE_DIR AND OPENEXR_HALF_LIBRARY AND OPENEXR_IEX_LIBRARY AND OPENEXR_ILMIMF_LIBRARY)
include_directories(SYSTEM ${OPENEXR_INCLUDE_DIR})
set(LIBS ${LIBS} ${OPENEXR_HALF_LIBRARY} ${OPENEXR_IEX_LIBRARY} ${OPENEXR_ILMIMF_LIBRARY})

# we will need the RPATHs set
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${PLUGINS_INSTALL_PATH}")


###########################################################
# BUILD

include_directories(src/libs)
add_subdirectory(src)


unset(CMAKE_BUILD_TYPE CACHE)

