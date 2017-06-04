cmake_minimum_required(VERSION 3.3)
project(A1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -D_BSD_SOURCE -DDEBUG")

set(SOURCE_FILES structs.h net_util.cpp)
add_executable(addpeer ${SOURCE_FILES} addpeer.cpp)
add_executable(removepeer ${SOURCE_FILES} removepeer.cpp)
add_executable(ping ${SOURCE_FILES} ping.cpp)
add_executable(addcontent ${SOURCE_FILES} addcontent.cpp)

