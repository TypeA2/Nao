// SPDX-License-Identifier: LGPL-3.0-or-later

#include "naofs.hpp"

#include <fcntl.h>

#include <fmt/format.h>

naofs::naofs(std::string_view source, archive_mode mode)
    : _mode { mode }, _path { fs::canonical(source) } {
    if (!fs::exists(_path)) {
        throw std::runtime_error(fmt::format("\"{}\" does not exist!", source));
    }

    if (!fs::is_regular_file(_path)) {
        throw std::runtime_error(fmt::format("Source must be a regular file (for now)"));
    }

    struct fd_raii {
        int fd;
        ~fd_raii() {
            close(fd);
        }
    } fd { open(_path.c_str(), O_RDONLY) };

    if (fd.fd == -1) {
        throw std::system_error(errno, std::generic_category(), "open");
    }

    _root_file = std::make_unique<mmapped_file>(fd.fd);
    _root = archive::resolve(std::string_view(_path.c_str()), *_root_file);
}

int naofs::getattr(std::string_view path, struct stat& stbuf) {
    (void) path;
    (void) stbuf;
    return 0;
}

int naofs::readdir(std::string_view path, off_t offset, fill_dir filler) {
    (void) offset;
    (void) path;
    filler(".", nullptr, 0);
    filler("..", nullptr, 0);
    filler("hello", nullptr, 0);

    return 0;
}
