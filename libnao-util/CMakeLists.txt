cmake_minimum_required(VERSION 3.8)

add_library(libnao-util STATIC
	"src/logging.cpp"
	"include/logging.h" "include/encoding.h")

set_property(TARGET libnao-util PROPERTY CXX_STANDARD 20)

target_include_directories(libnao-util PUBLIC "include")
target_compile_definitions(libnao-util PUBLIC UNICODE _UNICODE)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(libnao-util PUBLIC spdlog::spdlog)

include("FindIconv")
target_link_libraries(libnao-util PUBLIC Iconv::Iconv)
