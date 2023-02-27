// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/cripak.hpp"

#include <array>
#include <iostream>

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include "util/file_stream.hpp"

cripak_archive::cripak_archive(std::string_view name, file_stream& cripak_fs)
    : archive(cripak_fs) {

    spdlog::info("Opening .cpk: {}", name);

    std::array<char, 4> fourcc;
    if (fs.read(fourcc) != fourcc.size()) {
        throw io_error("unexpected EOF while reading magic number");
    }

    if (fourcc != std::array { 'C', 'P', 'K', ' ' }) {
        throw archive_error("unexpected magic number: expected 'CPK ', got \"{} {} {} {}\"",
            fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
    }

    if (uint32_t tag = fs.read_u32le(); tag != 0xFF) {
        throw archive_error("expected tag 0xFF, got {:#x}", tag);
    }

    [[maybe_unused]] uint64_t utf_size = fs.read_u64le();

    _cpk = std::make_unique<utf_table>(fs);

    if (_cpk->row_count() != 1) {
        throw archive_error("expected 1 row in CPK @UTF");
    }

    if (!_cpk->has_field("TocOffset")) {
        throw archive_error("TocOffset is required");
    }

    if (!_cpk->has_field("EtocOffset")) {
        throw archive_error("EtocOffset is required");
    } 

    auto toc_offset = _cpk->get<uint64_t>(0, "TocOffset");
    
    if (toc_offset > 2048) {
        spdlog::trace("Clamping toc_offset from {}", toc_offset);
        toc_offset = 2048;
    }

    auto etoc_offset = _cpk->get<uint64_t>(0, "EtocOffset");

    /* First 16 bytes of the archive are not counted */
    toc_offset  += 16;
    etoc_offset += 16;


    uint64_t extra_offset;
    if (!_cpk->has_field("ContentOffset")) {
        spdlog::trace("No ContentOffset");
        extra_offset = toc_offset;
    } else {
        auto content_offset = _cpk->get<uint64_t>(0, "ContentOffset");

        if (content_offset >= toc_offset) {
            extra_offset = toc_offset;

        } else {
            spdlog::trace("Used ContentOffset");
            extra_offset = content_offset;
        }
    }
    
    fs.seek(toc_offset);
    utf_table files(fs);

    for (uint32_t i = 0; i < files.row_count(); ++i) {
        auto dir_name = std::filesystem::path(files.get<std::string>(i, "DirName"));

        cripak_file file {
            .name         = files.get<std::string>(i, "FileName"),
            .file_size    = files.get<uint32_t>(i, "FileSize"),
            .extract_size = files.get<uint32_t>(i, "ExtractSize"),
            .file_offset  = files.get<uint64_t>(i, "FileOffset"),
            .id           = files.get<uint32_t>(i, "ID"),
            .user_string  = files.get<std::string>(i, "UserString"),
            .crc          = files.get<uint32_t>(i, "CRC"),
        };

        if (dir_name.empty()) {
            _root.files.push_back(file);
        } else {
            directory* cur = &_root;
            for (const auto& component : dir_name) {
                if (!cur->dirs.contains(component)) {
                    /* Insert component if needed */
                    cur->dirs[component] = directory{};
                }

                /* Move down the tree */
                cur = &cur->dirs[component];
            }

            cur->files.push_back(file);
        }
    }

    fs.seek(etoc_offset);
    // utf_table etoc(fs);
    // etoc.row_count();
}

void cripak_archive::contents(fill_func filler) {
    for (const cripak_file& file : _root.files) {
        filler(file.name, file_type::file);
    }

    for (const auto& [dir, _] : _root.dirs) {
        filler(dir, file_type::dir);
    }
}

bool cripak_archive::contains_archive(const std::filesystem::path& path) {
    for (const auto& [dir, _] : _root.dirs) {
        if (dir == path) {
            return true;
        }
    }

    return false;
}

archive& cripak_archive::get_archive(const std::filesystem::path& path) {
    throw std::runtime_error("not yet implemented");
}

int cripak_archive::stat(const std::filesystem::path& path, struct stat& stbuf) {
    for (const cripak_file& file : _root.files) {
        if (file.name == path.filename()) {
            stbuf.st_mode = 0544 | S_IFREG;
            stbuf.st_size = file.extract_size;
            return 0;
        }
    }

    for (const auto& [dirname, dir] : _root.dirs) {
        if (dirname == path.filename()) {
            stbuf.st_mode = 0755 | S_IFDIR;
            return 0;
        }
    }

    return -ENOENT;
}
