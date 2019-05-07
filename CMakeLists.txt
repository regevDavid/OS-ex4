cmake_minimum_required(VERSION 3.14)
project(ex4 C)

set(CMAKE_C_STANDARD 99)

add_executable(ex4 test.c osqueue.h osqueue.c threadPool.h threadPool.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")