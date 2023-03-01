// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/archive.hpp"
#include "archive/cripak.hpp"
#include "archive/directory.hpp"

#include "util/file_stream.hpp"

#include <fmt/format.h>

file_archive::file_archive(std::unique_ptr<file_stream> fs) : _fs { std::move(fs) }, fs { *_fs } { }

bool archive::resolve(const std::filesystem::path& path, std::unique_ptr<file_stream> fs, std::unique_ptr<archive>* archive) {
    if (is_directory(path)) {
        if (archive) {
            *archive = std::make_unique<directory_archive>(path);
        }

        return true;
    }

    /* Non-directory archive actually being opened requires file stream */
    if (archive && !fs) {
        return false;
    }

    if (path.extension() == ".cpk") {
        if (archive) {
            *archive = std::make_unique<cripak_archive>(path.filename().c_str(), std::move(fs));
        }

        return true;
    }

    return false;
}
