# Looking for boost
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

# Building the library
include_directories(./)

file(GLOB_RECURSE sources *.cpp)

add_library(dependency_graph SHARED ${sources})
install(TARGETS dependency_graph DESTINATION lib)

target_link_libraries(dependency_graph ${LIBS})
