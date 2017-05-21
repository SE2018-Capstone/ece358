cmake_minimum_required(VERSION 3.3)
project(A1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_BSD_SOURCE -DDEBUG")

set(SOURCE_FILES addpeer.c structs.h removepeer.c net_util.c)
add_executable(A1 ${SOURCE_FILES})
