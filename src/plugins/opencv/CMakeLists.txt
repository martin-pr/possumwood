if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
add_definitions(-Wno-error=ignored-optimization-argument) # not supported by clang
endif()

# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# Find opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OpenCV_LIBS})

# Find OpenImageIO using a custom module
include(possumwood_oiio)

# eigen
find_package(Eigen3 REQUIRED)
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

# tbb
pkg_search_module(TBB REQUIRED tbb)
include_directories(${TBB_INCLUDE_DIRS})
set(LIBS ${LIBS} ${TBB_LIBRARIES})

include_directories(./)

#####

file(GLOB sources *.cpp datatypes/*.cpp ui/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_opencv SHARED ${sources} ${headers_moc})

install(TARGETS psw_opencv DESTINATION lib)

target_link_libraries(psw_opencv ${LIBS} possumwood_sdk lightfields)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(opencv SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(opencv PROPERTIES PREFIX "")

install(TARGETS opencv DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(opencv
	psw_opencv
	psw_render
	${LIBS}
)
