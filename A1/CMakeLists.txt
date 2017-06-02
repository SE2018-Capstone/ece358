cmake_minimum_required(VERSION 3.3)
project(A1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_BSD_SOURCE -DDEBUG")

set(SOURCE_FILES addpeer.cpp structs.h removepeer.cpp net_util.cpp)
add_executable(A1 ${SOURCE_FILES})
