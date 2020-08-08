# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
set(LIBS ${LIBS} Qt5::Widgets)

add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)

include_directories(./)
include_directories(../)

#####

file(GLOB headers  *.h datatypes/*.h ui/*.h io/*.h)
file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp io/*.cpp)

QT5_WRAP_CPP(headers_moc ${headers})
add_definitions(${QT_DEFINITIONS})

add_library(psw_maths SHARED ${sources} ${headers_moc})

target_link_libraries(psw_maths ${LIBS} possumwood_sdk)

install(TARGETS psw_maths DESTINATION lib)

#####

file(GLOB_RECURSE node_sources nodes/*.cpp)

add_library(maths SHARED ${node_sources})
SET_TARGET_PROPERTIES(maths PROPERTIES PREFIX "")

install(TARGETS maths DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(maths psw_maths ${LIBS})
