include_directories(./)
include_directories(../../apps)

file(GLOB sources *.cpp)
list(APPEND sources ../../apps/possumwood/undo_stack.cpp)

message(${LIBS})

add_executable(possumwood_tests ${sources})

target_link_libraries(possumwood_tests ${LIBS} possumwood_sdk dependency_graph)
