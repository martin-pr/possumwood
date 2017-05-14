# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
include_directories(${QT_INCLUDE_DIR})
set(LIBS ${LIBS} Qt5::Widgets)

add_definitions(${QT_DEFINITIONS})

# Build
file(GLOB sources_sandbox *.cpp)
include_directories(./)

add_executable(nodeeditor_sandbox ${sources_sandbox})

target_link_libraries(nodeeditor_sandbox nodeeditor ${LIBS})
