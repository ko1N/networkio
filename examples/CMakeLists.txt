# tcp
add_executable(tcp_server tcp/server.cpp)
target_link_libraries(tcp_server networkio)

add_executable(tcp_client tcp/client.cpp)
target_link_libraries(tcp_client networkio)

# udp
add_executable(udp_server udp/server.cpp)
target_link_libraries(udp_server networkio)

add_executable(udp_client udp/client.cpp)
target_link_libraries(udp_client networkio)

if (BOTAN_FOUND)
  # tls
  add_executable(tls_server tls/server.cpp)
  target_link_libraries(tls_server networkio)

  add_executable(tls_client tls/client.cpp)
  target_link_libraries(tls_client networkio)
endif()

# mux
add_executable(mux_server mux/server.cpp)
target_link_libraries(mux_server networkio)

add_executable(mux_client mux/client.cpp)
target_link_libraries(mux_client networkio)

# sendfile
add_executable(sendfile_server sendfile/server.cpp)
target_link_libraries(sendfile_server networkio)

add_executable(sendfile_client sendfile/client.cpp)
target_link_libraries(sendfile_client networkio)

# http
add_executable(http_server http/server.cpp)
target_link_libraries(http_server networkio)

add_executable(http_login http/login.cpp)
target_link_libraries(http_login networkio)

add_executable(http_client http/client.cpp)
target_link_libraries(http_client networkio)
