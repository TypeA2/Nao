// SPDX-License-Identifier: LGPL-3.0-or-later

#include "naofs.hpp"

#include <fmt/format.h>

naofs::naofs(std::string_view source, archive_mode mode)
    : mode { mode }, path { fs::canonical(source) } {
    if (!fs::exists(path)) {
        throw std::runtime_error(fmt::format("\"{}\" does not exist!", source));
    }

    if (!fs::is_regular_file(path)) {
        throw std::runtime_error(fmt::format("Source must be a regular file (for now)"));
    }
}

int naofs::getattr(std::string_view path, struct stat& stbuf) {

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
