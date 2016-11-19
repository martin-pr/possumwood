project (animation_sandbox)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 2.8.4)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(LIBS "")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_definitions(-Wall -Werror -std=c++11 -g)

###########################################################
# DEPENDENCIES

# looking for boost
find_package(Boost REQUIRED COMPONENTS filesystem system serialization unit_test_framework program_options regex thread)
include_directories(${Boost_INCLUDE_DIR})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# blindly adding tbb
# set(LIBS ${LIBS} tbb)

###########################################################
# BUILD

include_directories(src)
add_subdirectory(src)
add_subdirectory(tests)
