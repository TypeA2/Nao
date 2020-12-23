cmake_minimum_required(VERSION 3.8)

add_library(libnao-ui STATIC
	"src/window.cpp"
	"include/window.h"
	"include/message_loop.h" "src/message_loop.cpp")

set_property(TARGET libnao-ui PROPERTY CXX_STANDARD 20)

target_include_directories(libnao-ui PUBLIC "include")
target_compile_definitions(libnao-ui PUBLIC UNICODE _UNICODE)
target_compile_options(libnao-ui PUBLIC /utf-8)

target_link_libraries(libnao-ui PUBLIC libnao-util)

