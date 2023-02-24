# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

# Add vcpkg toolchain file dynamically
find_program(VCPKG_EXECUTABLE vcpkg)
if(VCPKG_EXECUTABLE)
    cmake_path(GET VCPKG_EXECUTABLE PARENT_PATH VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(NaoFS VERSION 1.0.0)

add_executable(naofs
    "src/naofs.cpp"
)

set_target_properties(naofs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    INTERPROCEDURAL_OPTIMIZATION ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

find_package(cxxopts CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_include_directories(naofs PRIVATE "include")
target_compile_options(naofs PRIVATE "-Wall" "-Wextra" "-pedantic")

target_link_libraries(naofs PRIVATE
    cxxopts::cxxopts
    magic_enum::magic_enum
    fmt::fmt
    spdlog::spdlog
    range-v3 range-v3-meta range-v3::meta range-v3-concepts
)
