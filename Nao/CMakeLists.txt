cmake_minimum_required(VERSION 3.8)

add_executable(Nao WIN32
	"src/Nao.cpp"
	"include/nao_window.h"  "src/nao_window.cpp"
	"include/left_window.h" "src/left_window.cpp"
)

set_property(TARGET Nao PROPERTY CXX_STANDARD 20)
set_property(TARGET Nao PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(Nao PRIVATE "include")
target_link_libraries(Nao PRIVATE libnao-ui)

