cmake_minimum_required(VERSION 3.16)

project("jzip")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(huffman.out huffman_tree.cpp)
add_executable(jzip.out jzip.cpp)
