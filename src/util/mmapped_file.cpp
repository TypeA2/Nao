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

ssize_t mmapped_file::read(std::span<std::byte> dest) {
    if (pos >= _size) {
        return EOF;
    }

    size_t to_read = std::min<size_t>(dest.size_bytes(), _size - pos);

    std::copy_n(_addr + pos, to_read, dest.begin());

    return to_read;
}
