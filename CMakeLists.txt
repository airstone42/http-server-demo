cmake_minimum_required(VERSION 3.15)
project(http_server_demo C)

set(CMAKE_C_STANDARD 11)

add_executable(http_server_demo main.c server.c server.h)

target_link_libraries(http_server_demo pthread)
