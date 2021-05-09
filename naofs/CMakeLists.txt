cmake_minimum_required(VERSION 3.8)

add_executable(naofs
	"src/naofs.cpp"
     "include/projection.h" "src/projection.cpp" )

set_property(TARGET naofs PROPERTY CXX_STANDARD 20)
set_property(TARGET naofs PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDebug)

target_include_directories(naofs PRIVATE "include")
target_link_libraries(naofs PRIVATE libnao-util)

