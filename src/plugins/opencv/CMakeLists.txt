# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# Find opencv
find_package(OpenCV REQUIRED)
set(LIBS ${LIBS} ${OpenCV_LIBS})

# pkg-config to find jsoncpp
find_package(PkgConfig REQUIRED)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

include_directories(${JSONCPP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JSONCPP_LIBRARIES})

# Find OpenImageIO using a custom module
include(possumwood_oiio)

# eigen
find_package(Eigen3 REQUIRED)
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_opencv SHARED ${sources} ${headers_moc})

install(TARGETS psw_opencv DESTINATION lib)

target_link_libraries(psw_opencv ${LIBS} possumwood_sdk)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(opencv SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(opencv PROPERTIES PREFIX "")

install(TARGETS opencv DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(opencv
	psw_opencv
	psw_render
	${LIBS}
	OpenImageIO
	lightfields
)
