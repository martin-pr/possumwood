# Find Qt
SET(QT_USE_QTOPENGL TRUE)
FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)

######################################

file(GLOB_RECURSE sources *.cpp)
file(GLOB_RECURSE headers *.h)

QT5_WRAP_CPP(headers_moc ${headers})
include_directories(./ ../cgal)

add_library(anim SHARED ${sources} ${headers_moc})
install(TARGETS anim DESTINATION ${PLUGINS_INSTALL_PATH})
set_property(TARGET anim PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(anim possumwood_cgal ${LIBS} possumwood_sdk)
