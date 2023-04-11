// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/cripak.hpp"

#include <array>
#include <concepts>
#include <ctime>
#include <algorithm>

#include <sys/stat.h>

#include <spdlog/spdlog.h>
#include <magic_enum.hpp>

#include "util/file_stream.hpp"
#include "util/partial_file.hpp"

void cripak_archive::directory::contents(fill_func filler) {
    for (const cripak_file& file : files) {
        filler(file.name, file.archive ? file_type::dir : file_type::file);
    }

    for (const auto& [dir, _] : dirs) {
        filler(dir, file_type::dir);
    }
}

bool cripak_archive::directory::contains_archive(std::string_view name) {
    for (const cripak_file& file : files) {
        if ((file.name == name) && file.archive) {
            return true;
        }
    }

    for (const auto& [dir, _] : dirs) {
        if (dir == name) {
            return true;
        }
    }

    return false;
}

archive& cripak_archive::directory::get_archive(std::string_view name) {
    for (const cripak_file& file : files) {
        if ((file.name == name) && file.archive) {
            return *file.archive;
        }
    }

    for (auto& [dir, target] : dirs) {
        if (dir == name) {
            return target;
        }
    }

    throw archive_error("sub-archive \"{}\" not found", name);
}

int cripak_archive::directory::stat(std::string_view name, struct stat& stbuf) {
    for (const cripak_file& file : files) {
        if (file.name == name) {
            if (file.archive) {
                stbuf.st_mode = 0755 | S_IFDIR;
            } else {
                stbuf.st_mode = 0444 | S_IFREG;
                stbuf.st_size = file.extract_size;
                stbuf.st_mtime = file.update_datetime;
            }

            return 0;
        }
    }

    for (const auto& [dirname, dir] : dirs) {
        if (dirname == name) {
            stbuf.st_mode = 0755 | S_IFDIR;
            return 0;
        }
    }

    return -ENOENT;
}

int cripak_archive::directory::open(std::string_view name, int flags) {
    (void) flags;

    /* File must exist and be an actual file  */
    auto file_it = std::find_if(files.begin(), files.end(), [name](const cripak_file& file) {
        return file.name == name;
    });

    if (file_it == files.end()) {
        return -ENOENT;
    }

    cripak_file& file = *file_it;

    if (file.extract_size != file.file_size) {
        spdlog::error("Compressed files not supported (yet)");
        return -EACCES;
    }

    /* Well duh */
    if (!file.stream) {
        return -EISDIR;
    }

    return 0;
}


int cripak_archive::directory::read(std::string_view name, std::span<std::byte> buf, off_t offset) {
    auto file_it = std::find_if(files.begin(), files.end(), [name](const cripak_file& file) {
        return file.name == name;
    });

    if (file_it == files.end()) {
        return -ENOENT;
    }

    cripak_file& file = *file_it;

    /* Let file stream handle range checks */
    if (!file.stream) {
        return -EIO;
    }

    file.stream->seek(offset);
    return file.stream->read(buf);
}

cripak_archive::cripak_archive(std::string_view name, std::unique_ptr<file_stream> cripak_fs)
    : file_archive(std::move(cripak_fs)) {

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

    if (!_cpk->has_field("ContentOffset")) {
        spdlog::trace("No ContentOffset");
        _content_offset = toc_offset;
    } else {
        auto content_offset = _cpk->get<uint64_t>(0, "ContentOffset");

        if (content_offset >= toc_offset) {
            _content_offset = toc_offset;

        } else {
            spdlog::trace("Used ContentOffset");
            _content_offset = content_offset;
        }
    }
    
    /* Add 16 bytes to skip CPK header */
    fs.seek(toc_offset + 16);
    utf_table files(fs);

    fs.seek(etoc_offset + 16);
    utf_table etoc(fs);

    if (etoc.row_count() != (files.row_count() + 1)) {
        throw archive_error("expected {} Etoc rows, got {}", files.row_count() + 1, etoc.row_count());
    }

    for (uint32_t i = 0; i < files.row_count(); ++i) {
        auto dir_name = std::filesystem::path(files.get<std::string>(i, "DirName"));

        cripak_file file {
            .name         = files.get<std::string>(i, "FileName"),
            .file_size    = files.get<uint32_t>(i, "FileSize"),
            .extract_size = files.get<uint32_t>(i, "ExtractSize"),
            .file_offset  = files.get<uint64_t>(i, "FileOffset"),
            .id           = files.get<uint32_t>(i, "ID"),
            .crc          = files.get<uint32_t>(i, "CRC"),

            /* Etoc */
            .update_datetime = convert_datetime(etoc.get<uint64_t>(i, "UpdateDateTime")),
            .local_dir       = etoc.get<std::string>(i, "LocalDir"),
        };

        file.absolute_offset = file.file_offset + _content_offset;

        if (files.get_field("UserString").has_value()) {
            file.user_string = files.get<std::string>(i, "UserString");
        }

        if (file.file_size != file.extract_size) {
            spdlog::trace("compressed file: {} ({} -> {})", file.name, file.file_size, file.extract_size);
        } else {
            file.stream = std::make_unique<partial_file>(fs, file.absolute_offset, file.file_size);

            /* Sub-archive takes over ownership */
            if (archive::is_archive(file.name, file.stream.get())) {
                file.archive = archive::get_archive(name, std::move(file.stream));
                file.stream = nullptr;
            }
        }

        if (dir_name.empty()) {
            _root.files.emplace_back(std::move(file));
        } else {
            directory* cur = &_root;
            for (const auto& component : dir_name) {
                if (!cur->dirs.contains(component)) {
                    /* Insert component if needed */
                    cur->dirs.emplace(component, directory{});
                }

                /* Move down the tree */
                cur = &cur->dirs.at(component);
            }

            cur->files.emplace_back(std::move(file));
        }
    }
}

void cripak_archive::contents(fill_func filler) {
    _root.contents(std::move(filler));
}

bool cripak_archive::contains_archive(std::string_view name) {
    return _root.contains_archive(name);
}

archive& cripak_archive::get_archive(std::string_view name) {
    return _root.get_archive(name);
}

int cripak_archive::stat(std::string_view name, struct stat& stbuf) {
    return _root.stat(name, stbuf);
}

int cripak_archive::open(std::string_view name, int flags) {
    return _root.open(name, flags);
}

int cripak_archive::read(std::string_view name, std::span<std::byte> buf, off_t offset) {
    return _root.read(name, buf, offset);
}

uint64_t cripak_archive::convert_datetime(uint64_t cpk_datetime) {
    /* https://github.com/blueskythlikesclouds/SonicAudioTools/blob/master/Source/SonicAudioLib/Archives/CriCpkArchive.cs#L670 */

    std::tm tm;
    tm.tm_year = ((cpk_datetime >> 48) & 0xFFFF) - 1900; /* 1900-based */
    tm.tm_mon  = ((cpk_datetime >> 40) & 0xFF) - 1;      /* 0-based */
    tm.tm_mday =  (cpk_datetime >> 32) & 0xFF;
    tm.tm_hour =  (cpk_datetime >> 24) & 0xFF;
    tm.tm_min  =  (cpk_datetime >> 16) & 0xFF;
    tm.tm_sec  =  (cpk_datetime >>  8) & 0xFF;
    tm.tm_isdst = 0;
    tm.tm_wday  = 0;
    tm.tm_yday  = 0;

    return std::mktime(&tm);
}
