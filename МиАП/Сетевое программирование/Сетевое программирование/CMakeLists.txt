cmake_minimum_required(VERSION 3.10)
project(TCP_Client_Server)

set(CMAKE_CXX_STANDARD 17)

# Клиент
add_executable(tcp_client tcp_client.cpp)

# Сервер
add_executable(tcp_server tcp_server.cpp)
target_link_libraries(tcp_server pthread)