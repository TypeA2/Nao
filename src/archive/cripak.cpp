// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/cripak.hpp"

#include <array>
#include <iostream>

#include <fmt/ostream.h>

#include "util/file_stream.hpp"

cripak_archive::cripak_archive(std::string_view name, file_stream& cripak_fs)
    : archive(cripak_fs) {

    (void) name;

    std::array<char, 4> fourcc;
    if (fs.read(fourcc) != fourcc.size()) {
        throw io_error("unexpected EOF while reading magic number");
    }

    if (fourcc != std::array { 'C', 'P', 'K', ' ' }) {
        throw archive_error("unexpected magic number: \"{} {} {} {}\"",
            fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    }

    fmt::print(std::cerr, "end\n");
}
