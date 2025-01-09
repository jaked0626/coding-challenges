cmake_minimum_required(VERSION 3.16)

project("jzip")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(huffman huffman_tree.cpp)
add_executable(jzip jzip.cpp huffman_tree.cpp)

target_compile_definitions(huffman PRIVATE TEST_HUFFMAN_TREE)

set_target_properties(huffman PROPERTIES OUTPUT_NAME "huffman.out")
set_target_properties(jzip PROPERTIES OUTPUT_NAME "jzip.out")