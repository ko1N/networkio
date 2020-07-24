file(GLOB_RECURSE networkio_sources "src/*.cpp")
file(GLOB_RECURSE networkio_headers "inc/*.h")
group_sources("src")
group_sources("inc")

add_library(networkio STATIC ${networkio_sources} ${networkio_headers})
target_link_libraries(networkio ${NETWORKIO_LIBRARIES})
target_include_directories(networkio PUBLIC inc)

set_target_properties(networkio PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
configure_file(networkio.pc.in networkio.pc @ONLY)

install(TARGETS networkio ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/networkio.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
install(DIRECTORY inc/networkio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})