cmake_minimum_required(VERSION 3.8)

add_library(libnao-util STATIC
	"include/libnao/logging.h" "src/logging.cpp"
	"include/libnao/encoding.h" "src/encoding.cpp"
	"include/libnao/libnao_util.h" "src/libnao_util.cpp" )

set_property(TARGET libnao-util PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-util PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-util PUBLIC "include")
target_include_directories(libnao-util PRIVATE "include/libnao")
target_compile_definitions(libnao-util PUBLIC UNICODE _UNICODE)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(libnao-util PUBLIC spdlog::spdlog)

include("FindIconv")
target_link_libraries(libnao-util PUBLIC Iconv::Iconv)

target_link_options(libnao-util PUBLIC /entry:wWinMainCRTStartup)
