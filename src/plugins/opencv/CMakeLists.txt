# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

# Find opencv
find_package(OpenCV REQUIRED)
set(LIBS ${LIBS} ${OpenCV_LIBS})

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
)
