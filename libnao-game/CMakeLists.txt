cmake_minimum_required(VERSION 3.8)

add_library(libnao-game STATIC
	"include/libnao/libnao_game.h"       "src/libnao_game.cpp"
	"include/libnao/game/item_handler.h" "src/item_handler.cpp"
)

set_property(TARGET libnao-game PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-game PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-game PUBLIC "include")
target_include_directories(libnao-game PRIVATE
	"include/libnao"
	"include/libnao/game"
)

target_compile_definitions(libnao-game PUBLIC UNICODE _UNICODE NOMINMAX)
target_compile_options(libnao-game PUBLIC /utf-8)

target_link_libraries(libnao-game PUBLIC libnao-util)

