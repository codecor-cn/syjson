cmake_minimum_required(VERSION 2.6)
project(syjson_test C)

if(CMAKE_C_COMPILER_ID MATCHES "GUN|Clang")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --ansi -pedantic -Wall")
endif()

add_library(syjson syjson.c)
add_executable(syjson_test test.c)
target_link_libraries(syjson_test syjson)
