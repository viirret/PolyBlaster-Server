cmake_minimum_required(VERSION 3.16)
project(server)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DDEBUG)

include_directories(src)
file(GLOB sources src/*.cc)
add_executable(x ${sources})
