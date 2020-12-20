cmake_minimum_required (VERSION 3.8)

project("Nao")

add_executable(Nao WIN32
	"src/Nao.cpp"
)

set_property(TARGET Nao PROPERTY CXX_STANDARD 20)

target_include_directories(Nao PRIVATE "include")
#target_compile_definitions(Nao PRIVATE UNICODE _UNICODE)

add_subdirectory(libnao-ui)
target_link_libraries(Nao PRIVATE libnao-ui)


