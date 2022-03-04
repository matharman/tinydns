cmake_minimum_required(VERSION 3.16)
project(tiny_dns C CXX)

add_library(tiny_dns STATIC 
	lib/io.c
	lib/label.c
	lib/tiny_dns.c
	)
target_include_directories(tiny_dns PUBLIC lib)
target_compile_options(tiny_dns PRIVATE -Wall -Wpedantic -Wno-enum-compare -Werror -std=c99)
add_subdirectory(lib/rdata)

add_executable(tiny_dns_cli cli/main.c)
target_link_libraries(tiny_dns_cli PRIVATE tiny_dns)

enable_testing()
add_subdirectory(tests)
