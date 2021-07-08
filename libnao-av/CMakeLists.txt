cmake_minimum_required(VERSION 3.8)

add_library(libnao-av STATIC
	"include/libnao/libnao_av.h" "src/libnao_av.cpp"
	"include/libnao/av/image.h"  "src/image.cpp"
)

set_property(TARGET libnao-av PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-av PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-av PUBLIC "include")
target_include_directories(libnao-av PRIVATE
	"include/libnao"
	"include/libnao/av"
)

target_compile_definitions(libnao-av PUBLIC UNICODE _UNICODE NOMINMAX)
target_compile_options(libnao-av PUBLIC /utf-8)

target_link_libraries(libnao-av PUBLIC libnao-util)

