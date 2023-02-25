// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/mmapped_file.hpp"

#include <system_error>
#include <iostream>
#include <algorithm>

#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>

#include <fmt/ostream.h>

mmapped_file::mmapped_file(int fd) {
    struct stat st;
    if (fstat(fd, &st) != 0) {
        throw std::system_error(errno, std::generic_category(), "fstat");
    }
    
    _size = st.st_size;
    _addr = static_cast<std::byte*>(mmap(nullptr, _size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (_addr == MAP_FAILED) {
        throw std::system_error(errno, std::generic_category(), "mmap");
    }
}

mmapped_file::~mmapped_file() {
    if (_addr && _addr != MAP_FAILED && munmap(_addr, _size) != 0) {
        fmt::print(std::cerr, "munmap: {} ({})\n", strerror(errno), errno);
        std::cerr.flush();
        std::terminate();
    }
}

std::streamsize mmapped_file::size() const {
    return _size;
}

void mmapped_file::seek(std::streamoff new_pos) {
    if (pos < 0) {
        throw std::out_of_range("invalid negative seek");
    }

    if (new_pos >= _size) {
        throw std::out_of_range(fmt::format("position {} is out of range ({} >= {})", new_pos, new_pos, _size));
    }

    pos = new_pos;
}

std::streamsize mmapped_file::read(std::span<std::byte> dest) {
    if (pos >= _size) {
        throw io_error("EOF");
    }

    auto to_read = std::min<std::streamsize>(dest.size_bytes(), _size - pos);

    std::copy_n(_addr + pos, to_read, dest.begin());

    pos += to_read;

    return to_read;
}

std::byte mmapped_file::getb() {
    if (pos >= _size) {
        throw io_error("EOF");
    }

    pos += 1;
    return *(_addr + pos - 1);
}
