# Find Qt
SET(QT_USE_QTOPENGL TRUE)
find_package(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# using pkg-config
find_package(PkgConfig REQUIRED)

# Find luabind
pkg_check_modules (LUABIND luabind)

pkg_search_module(LUABIND REQUIRED)
include_directories(${LUABIND_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LUABIND_LIBRARIES})

# Find OpenImageIO using a custom module
include(possumwood_oiio)

# Find opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OpenCV_LIBS})

include_directories(./)

#####

file(GLOB sources *.cpp lua/*.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})

add_library(psw_images SHARED ${sources} ${headers_moc})

install(TARGETS psw_images DESTINATION lib)

target_link_libraries(psw_images psw_lua ${LIBS} possumwood_sdk)

#####

file(GLOB_RECURSE nodes_sources nodes/*.cpp)

add_library(images SHARED ${nodes_sources})
SET_TARGET_PROPERTIES(images PROPERTIES PREFIX "")

install(TARGETS images DESTINATION ${PLUGINS_INSTALL_PATH})

target_link_libraries(images psw_images psw_opencv lightfields ${OPENEXR_IEX_LIBRARY} ${LIBS})
