# Looking for boost
find_package(Boost REQUIRED COMPONENTS
	filesystem system
)
include_directories(${Boost_INCLUDE_DIR})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

# Building the library
include_directories(./)

file(GLOB_RECURSE sources *.cpp)

add_library(lightfields SHARED ${sources})
install(TARGETS lightfields DESTINATION lib)

target_link_libraries(lightfields ${LIBS})
