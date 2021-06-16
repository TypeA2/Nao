cmake_minimum_required(VERSION 3.8)

project("Nao")

set(NAO_VERSION 1.0.0)

add_subdirectory(libnao-util)
add_subdirectory(libnao-ui)
add_subdirectory(libnao-game)
# add_subdirectory(naofs)
add_subdirectory(Nao)
