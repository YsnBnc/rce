cmake_minimum_required(VERSION 3.15)
project(RPC VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define your executables
add_executable(host src/host.cpp)
add_executable(client src/client.cpp)

# Windows-specific network library requirement
if(WIN32)
    target_link_libraries(host PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
endif()