cmake_minimum_required(VERSION 3.16)
project(server)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/boost)
set(BOOST_STATIC true)

add_compile_options(-Wall -Wextra -pedantic -static-libstdc++ -static-libgcc -pthread)

add_definitions(-DDEBUG)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(websocketpp)
include_directories(src)
file(GLOB sources src/*.cc)
add_executable(x ${sources})

