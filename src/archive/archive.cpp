// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/archive.hpp"
#include "archive/directory.hpp"
#include "archive/cripak.hpp"
#include "archive/cridat.hpp"

#include "util/file_stream.hpp"

#include <fmt/format.h>

file_archive::file_archive(std::unique_ptr<file_stream> fs) : _fs { std::move(fs) }, fs { *_fs } { }

bool archive::is_archive(const std::filesystem::path& path, file_stream* fs) {
    return resolve_internal(path, fs, false).first;
}

std::unique_ptr<archive> archive::get_archive(const std::filesystem::path& path, std::unique_ptr<file_stream> fs) {
    /* This is slightly illegal:
     *   Release file pointer. This gets re-acquired immediately inside this function
     */
    if (auto res = resolve_internal(path, fs.release(), true); res.first) {
        return std::move(res.second);
    }

    throw std::runtime_error(fmt::format("Archive at {} not found", path.string()));
}

archive::resolve_type archive::resolve_internal(const std::filesystem::path& path, file_stream* fs, bool make_archive) {
    /* Acquire if we're actually going to use it, do nothing otherwise  */
    std::unique_ptr<file_stream> fptr { make_archive ? fs : nullptr };
    
    if (is_directory(path)) {
        if (make_archive && fs) {
            /* We were passed a file_stream to a real directory? panic */
            throw std::runtime_error(fmt::format("directory {} exists but has file contents!", path.string()));
        }

        return { true, make_archive ? std::make_unique<directory_archive>(path) : nullptr };
    }

    /* Only a directory may have linked file_stream */
    if (make_archive && !fs) {
        return { false, nullptr };
    }

    auto filename_string = path.filename().string();
    auto extension = path.extension();

    if (extension == ".cpk") {
        return { 
            true,
            make_archive ? std::make_unique<cripak_archive>(filename_string, std::move(fptr)) : nullptr
        };
    } else if (extension == ".dat" || extension == ".dtt") {
        return { 
            true,
            make_archive ? std::make_unique<cridat_archive>(filename_string, std::move(fptr)) : nullptr
        };
    }

    return { false, nullptr };
}
