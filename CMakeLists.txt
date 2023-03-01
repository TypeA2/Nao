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
    "src/main.cpp"
    "include/naofs.hpp"             "src/naofs.cpp"
    "include/archive/archive.hpp"   "src/archive/archive.cpp"
    "include/archive/cripak.hpp"    "src/archive/cripak.cpp"
    "include/archive/directory.hpp" "src/archive/directory.cpp"
    "include/util/file_stream.hpp"  "src/util/file_stream.cpp"
    "include/util/mmapped_file.hpp" "src/util/mmapped_file.cpp"
    "include/util/fstream_file.hpp" "src/util/fstream_file.cpp"
    "include/util/cripak_utf.hpp"   "src/util/cripak_utf.cpp"
    "include/util/exceptions.hpp"
)

set_target_properties(naofs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    # INTERPROCEDURAL_OPTIMIZATION ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# find_package(cxxopts CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(fuse3 REQUIRED IMPORTED_TARGET fuse3)

target_include_directories(naofs PRIVATE "include")
target_compile_options(naofs PRIVATE "-Wall" "-Wextra" "-pedantic" "-Wno-sign-compare")

target_link_libraries(naofs PRIVATE
    # cxxopts::cxxopts
    magic_enum::magic_enum
    fmt::fmt
    spdlog::spdlog
    range-v3 range-v3-meta range-v3::meta range-v3-concepts
    PkgConfig::fuse3
)
