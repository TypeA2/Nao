cmake_minimum_required(VERSION 3.8)

add_library(libnao-ui STATIC
	"include/libnao/window.h" "src/window.cpp"
	"include/libnao/message_loop.h" "src/message_loop.cpp"
	"include/libnao/main_window.h" "src/main_window.cpp"
	"include/libnao/push_button.h" "src/push_button.cpp"
	"include/libnao/libnao_ui.h" "src/libnao_ui.cpp" "include/libnao/layout.h" "src/layout.cpp" "include/libnao/horizontal_layout.h" "src/horizontal_layout.cpp" "include/libnao/event.h" "src/event.cpp"  )

set_property(TARGET libnao-ui PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-ui PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-ui PUBLIC "include")
target_include_directories(libnao-ui PRIVATE "include/libnao")
target_compile_definitions(libnao-ui PUBLIC UNICODE _UNICODE)
target_compile_options(libnao-ui PUBLIC /utf-8)

target_link_libraries(libnao-ui PUBLIC libnao-util)
target_link_libraries(libnao-ui PRIVATE Comctl32.lib)
