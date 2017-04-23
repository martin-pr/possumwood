SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})
set(LIBS ${LIBS} ${QT_LIBRARIES})
message(STATUS ${QT_LIBRARIES})


include_directories(./)

file(GLOB sources_sandbox *.cpp)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(nodeeditor_sandbox ${sources_sandbox})

target_link_libraries(nodeeditor_sandbox nodeeditor ${LIBS})
