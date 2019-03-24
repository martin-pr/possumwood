project (possumwood)

# set(CMAKE_CXX_COMPILER "clang")

cmake_minimum_required (VERSION 2.8.4)
# set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-Wall -Werror -g)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)
add_definitions(-Wl,--no-undefined)

# required by CGAL
add_definitions(-DBOOST_PARAMETER_MAX_ARITY=12)


# DNEG-specific hack, should have no impact for anything else
include_directories($ENV{BOB_WORLD_SLOT_ilmbase}/include/OpenEXR)

###########################################################
# DEPENDENCIES

set(LIBS "")

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


###########################################################
# INSTALL TARGETS

message("***** Install prefix = ${CMAKE_INSTALL_PREFIX} *****")

set(POSSUMWOOD_CONF_PATH ${CMAKE_INSTALL_PREFIX}/etc/possumwood.conf)
set(PLUGINS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/possumwood/plugins)
set(EXAMPLES_INSTALL_PATH ${POSSUMWOOD_DATA_PREFIX}/examples)
set(TOOLBARS_INSTALL_PATH ${POSSUMWOOD_DATA_PREFIX}/toolbars)

add_custom_target(
	config ALL
	COMMAND bash -cx \"echo \\\"EXAMPLES ${EXAMPLES_INSTALL_PATH}\\nTOOLBARS ${TOOLBARS_INSTALL_PATH}\\nPLUGINS ${PLUGINS_INSTALL_PATH}\\n\\\" > \\\"${CMAKE_CURRENT_BINARY_DIR}/possumwood.conf\\\"\"
)

message("  Generated config file: ${CMAKE_CURRENT_BINARY_DIR}/possumwood.conf")

set(POSSUMWOOD_DATA_PREFIX "${CMAKE_INSTALL_PREFIX}/share/possumwood")



message("  Master config file: ${POSSUMWOOD_CONF_PATH}")
message("  Plugins directory: ${PLUGINS_INSTALL_PATH}")
message("  Examples directory: ${EXAMPLES_INSTALL_PATH}")
message("  Toolbars directory: ${TOOLBARS_INSTALL_PATH}")

# pass the config file explicitly to C++
add_definitions(-DPOSSUMWOOD_CONF_PATH=${POSSUMWOOD_CONF_PATH})

# generate the config file content
#install(CODE "file(WRITE \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/etc/possumwood.conf\" \"EXAMPLES ${EXAMPLES_INSTALL_PATH}\nTOOLBARS ${TOOLBARS_INSTALL_PATH}\nPLUGINS ${PLUGINS_INSTALL_PATH}\n\")")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/possumwood.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)

# copy all toolbars and examples
FILE(GLOB TOOLBARS "toolbars/*")
install(DIRECTORY ${TOOLBARS} DESTINATION ${TOOLBARS_INSTALL_PATH})

install(DIRECTORY examples DESTINATION ${POSSUMWOOD_DATA_PREFIX})

# we will need the RPATHs set
# SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${PLUGINS_INSTALL_PATH}")


###########################################################
# BUILD

include_directories(src/libs)
add_subdirectory(src)
