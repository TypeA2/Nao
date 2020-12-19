cmake_minimum_required (VERSION 3.8)

project("Nao")

add_executable(Nao
	"src/Nao.cpp"
	"include/Nao.h"
)

set_property(TARGET Nao PROPERTY CXX_STANDARD 20)

target_include_directories(Nao PRIVATE "include")

add_subdirectory(libnao-ui)
target_link_libraries(Nao PRIVATE libnao-ui)


