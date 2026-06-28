cmake_minimum_required(VERSION 3.15)
project(RPC VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(file_manager_lib STATIC
        src/file_manager.cpp
)

# Define your executables
add_executable(host src/host.cpp)
add_executable(client
        src/client.cpp
        src/file_manager.cpp)
target_link_libraries(client PRIVATE file_manager_lib)

add_executable(file_manager src/file_manager.cpp)

# Windows-specific network library requirement
if(WIN32)
    target_link_libraries(host PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
endif()