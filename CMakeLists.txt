cmake_minimum_required(VERSION 3.15)
project(RCE VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(RCE
        src/main.cpp
        src/utilities/client_side.cpp
        src/utilities/server_side.cpp
        src/utilities/file_manager.cpp
)

find_package(wxWidgets CONFIG REQUIRED)
target_link_libraries(RCE PRIVATE wx::wxbase wx::wxcore)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    target_link_libraries(RCE PRIVATE ws2_32)
endif()
