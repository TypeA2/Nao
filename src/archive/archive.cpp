// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/archive.hpp"
#include "archive/cripak.hpp"

#include "util/file_stream.hpp"

#include <fmt/format.h>

archive::archive(file_stream& fs) : fs { fs } { }

std::unique_ptr<archive> archive::resolve(std::string_view name, file_stream& fs) {
    if (name.ends_with(".cpk")) {
        return std::make_unique<cripak_archive>(name, fs);
    }

    return nullptr;
}
