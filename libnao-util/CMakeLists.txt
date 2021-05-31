cmake_minimum_required(VERSION 3.8)

add_library(libnao-util STATIC
	"include/libnao/libnao_util.h"   "src/libnao_util.cpp"
	"include/libnao/util/logging.h"  "src/logging.cpp"
	"include/libnao/util/encoding.h" "src/encoding.cpp"
	"include/libnao/util/ranges.h"
	"include/libnao/util/traits.h"
	"include/libnao/util/concepts.h"
	"include/libnao/util/_win32_formatters.h" 
	"include/libnao/util/win32.h"          "src/win32.cpp"
	"include/libnao/util/shared_library.h" "src/shared_library.cpp")

set_property(TARGET libnao-util PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-util PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-util PUBLIC "include")
target_include_directories(libnao-util PRIVATE "include/libnao")
target_include_directories(libnao-util PRIVATE "include/libnao/util")
target_compile_definitions(libnao-util PUBLIC UNICODE _UNICODE)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(libnao-util PUBLIC spdlog::spdlog)

include("FindIconv")
target_link_libraries(libnao-util PUBLIC Iconv::Iconv)

target_link_options(libnao-util PUBLIC /entry:wWinMainCRTStartup)
