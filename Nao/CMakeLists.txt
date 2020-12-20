cmake_minimum_required(VERSION 3.8)

add_executable(Nao WIN32
	"src/Nao.cpp"
)

set_property(TARGET Nao PROPERTY CXX_STANDARD 20)

target_include_directories(Nao PRIVATE "include")
target_link_libraries(Nao PRIVATE libnao-ui)


