project (possumwood CXX)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 3.0.0)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# this is an anti-pattern, but CMAKE does seem to be picking up CMAKE_CXX_FLAGS_RELEASE and CMAKE_CXX_FLAGS_DEBUG
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function")
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-function")
elseif(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-function")
else()
	message(ERROR "Unknown build type.")
endif()

message("*** BUILDING ${CMAKE_BUILD_TYPE} BUILD OF POSSUMWOOD ***")
message("- C++ flags: ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "/usr/share/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # allow custom modules

add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--no-undefined")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,--no-undefined")

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

# setup the OpenGL preference for CMake, to fix CMP0072 warning
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# looking for OpenGL libraries
#find_package(OpenGL REQUIRED)
#include_directories(${OPENGL_INCLUDE_DIR})
#set(LIBS ${LIBS} ${OPENGL_LIBRARY})

# looking for glut
#find_package(GLUT REQUIRED)
#include_directories(${GLUT_INCLUDE_DIR})
#set(LIBS ${LIBS} ${GLUT_LIBRARY})

# looking for GLFW
find_package(glfw3 REQUIRED)
include_directories(${glfw3_INCLUDE_DIR})

# blindly adding tbb
# set(LIBS ${LIBS} tbb)

# looking for openexr libs
find_package(PkgConfig REQUIRED)

pkg_search_module(ILMBASE REQUIRED IlmBase)
include_directories(${ILMBASE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${ILMBASE_LIBRARIES})

pkg_search_module(OPENEXR REQUIRED OpenEXR)
include_directories(${OPENEXR_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENEXR_LIBRARIES})

# we will need the RPATHs set
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${PLUGINS_INSTALL_PATH}")


###########################################################
# BUILD

include_directories(src/libs)
add_subdirectory(src)


unset(CMAKE_BUILD_TYPE CACHE)

