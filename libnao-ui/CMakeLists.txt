cmake_minimum_required(VERSION 3.8)

add_library(libnao-ui STATIC
	"include/libnao/libnao_ui.h"              "src/libnao_ui.cpp"
	"include/libnao/ui/defs.h"                "src/defs.cpp"
	"include/libnao/ui/window.h"              "src/window.cpp"
	"include/libnao/ui/message_loop.h"        "src/message_loop.cpp"
	"include/libnao/ui/main_window.h"         "src/main_window.cpp"
	"include/libnao/ui/layout.h"              "src/layout.cpp"
	"include/libnao/ui/directional_layout.h"  "src/directional_layout.cpp"
	"include/libnao/ui/event.h"               "src/event.cpp"
	"include/libnao/ui/icon.h"                "src/icon.cpp"
	"include/libnao/ui/push_button.h"         "src/push_button.cpp"
	"include/libnao/ui/line_edit.h"           "src/line_edit.cpp"
)

set_property(TARGET libnao-ui PROPERTY CXX_STANDARD 20)
set_property(TARGET libnao-ui PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(libnao-ui PUBLIC "include")
target_include_directories(libnao-ui PRIVATE
	"include/libnao"
	"include/libnao/ui"
)

target_compile_definitions(libnao-ui PUBLIC UNICODE _UNICODE NOMINMAX)
target_compile_options(libnao-ui PUBLIC /utf-8)

target_link_libraries(libnao-ui PUBLIC libnao-util)
target_link_libraries(libnao-ui PRIVATE Comctl32.lib)
