# tests/packet
file(GLOB_RECURSE test_packet_sources "packet/*.cpp")
add_executable(test_packet ${test_packet_sources})
target_link_libraries(test_packet networkio ${NETWORKIO_LIBRARIES})
add_test(NAME test_packet COMMAND test_packet WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# tests/mux
file(GLOB_RECURSE test_mux_sources "mux/*.cpp")
add_executable(test_mux ${test_mux_sources})
target_link_libraries(test_mux networkio ${NETWORKIO_LIBRARIES})
add_test(NAME test_mux COMMAND test_mux WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
