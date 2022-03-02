cmake_minimum_required(VERSION 3.16)
project(tiny_dns C CXX)

add_library(tiny_dns STATIC lib/io.c)
target_include_directories(tiny_dns PUBLIC lib)

enable_testing()
add_subdirectory(tests)
