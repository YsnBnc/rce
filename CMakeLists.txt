cmake_minimum_required(VERSION 3.15)
project(RCE VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RCE
        src/main.cpp
        src/utilities/client.cpp
        src/utilities/host.cpp
        src/utilities/file_manager.cpp)

if(WIN32)
    target_link_libraries(RCE ws2_32)
endif()