cmake_minimum_required (VERSION 3.8)

project ("libnao-ui")

add_library(libnao-ui STATIC
	"src/libnao-ui.cpp"
	"include/libnao-ui.h"
	"src/window.cpp"
	"include/window.h"
	)

set_property(TARGET libnao-ui PROPERTY CXX_STANDARD 20)

target_include_directories(libnao-ui PUBLIC "include")
target_compile_definitions(libnao-ui PUBLIC UNICODE _UNICODE)
