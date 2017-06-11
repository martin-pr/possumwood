# looking for openmesh libs
SET(OPENMESH_LIBRARY_PATHS /usr/lib /usr/local/lib /sw/lib /opt/local/lib)
FIND_PATH(OPENMESH_INCLUDE_DIR Core/Mesh/BaseMesh.hh PATH_SUFFIXES OpenMesh /usr/include /usr/local/include /sw/include /opt/local/include)
FIND_LIBRARY(OPENMESH_LIBRARY NAMES OpenMeshCore PATHS ${OPENMESH_LIBRARY_PATHS})
if (OPENMESH_INCLUDE_DIR AND OPENMESH_LIBRARY)
  set (OPENMESH_FOUND TRUE)
  message (STATUS "OpenMesh library found (${OPENMESH_INCLUDE_DIR} and ${OPENMESH_LIBRARY}).")
else (OPENMESH_INCLUDE_DIR AND OPENMESH_LIBRARY)
  message (FATAL_ERROR "OpenMesh library not found (${OPENMESH_INCLUDE_DIR} and ${OPENMESH_LIBRARY}).")
endif (OPENMESH_INCLUDE_DIR AND OPENMESH_LIBRARY)
include_directories(${OPENMESH_INCLUDE_DIR})
set(LIBS ${LIBS} ${OPENMESH_LIBRARY})

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)

include_directories(./)

add_library(openmesh SHARED ${sources})

target_link_libraries(openmesh ${LIBS})
