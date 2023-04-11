// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archive/cridat.hpp"

#include <spdlog/spdlog.h>

#include "util/file_stream.hpp"
#include "util/partial_file.hpp"

cridat_archive::cridat_archive(std::string_view name, std::unique_ptr<file_stream> cridat_fs)
    : file_archive(std::move(cridat_fs)) {

    spdlog::info("Opening .dat/.dtt: {}", name);

    std::array<char, 4> fourcc;
    if (fs.read(fourcc) != fourcc.size()) {
        throw io_error("unexpected EOF while reading magic number");
    }

    if (fourcc != std::array { 'D', 'A', 'T', '\0' }) {
        throw archive_error("unexpected magic number: expected 'DAT\0', got \"{} {} {} {}\"",
            fourcc[0], fourcc[1], fourcc[2], static_cast<int>(fourcc[3]));
    }

    /* General info */
    uint32_t file_count = fs.read_u32le();
    uint32_t offset_table_offset = fs.read_u32le();
    [[maybe_unused]] uint32_t extension_table_offset = fs.read_u32le();
    uint32_t name_table_offset = fs.read_u32le();
    uint32_t size_table_offset = fs.read_u32le();

    std::vector<std::pair<std::string, cridat_file>> files(file_count);

    /* Data is spread across multiple tables */
    fs.seek(name_table_offset);
    uint32_t name_alignment = fs.read_u32le();
    std::vector<std::byte> name_buf(name_alignment);
    for (auto& [name, file] : files) {
        std::streamoff start = fs.tell();

        name = fs.read_ncstring(name_alignment);

        fs.seek(start + name_alignment);
    }

    fs.seek(size_table_offset);
    for (auto& [name, file] : files) {
        file.size = fs.read_u32le();
    }


    fs.seek(offset_table_offset);
    for (auto& [name, file] : files) {
        file.offset = fs.read_u32le();
    }

    for (auto& [name, file] : files) {
        auto stream = std::make_unique<partial_file>(fs, file.offset, file.size);

        if (archive::is_archive(name, stream.get())) {
            file.archive = archive::get_archive(name, std::move(stream));
        } else {
            file.fs = std::move(stream);
        }

        _files.emplace(name, std::move(file));
    }
}

void cridat_archive::contents(fill_func filler) {
    for (const auto& [name, file] : _files) {
        filler(name, file.archive ? file_type::dir : file_type::file);
    }
}

bool cridat_archive::contains_archive(std::string_view name) {
    auto it = _files.find(name);
    return (it != _files.end()) && it->second.archive;
}

archive& cridat_archive::get_archive(std::string_view name) {
    auto it = _files.find(name);
    if (it == _files.end() || !it->second.archive) {
        throw archive_error("Archive {} does not exist", name);
    }

    return *it->second.archive;
}

int cridat_archive::stat(std::string_view name, struct stat& stbuf) {
    auto it = _files.find(name);
    if (it == _files.end()) {
        return -ENOENT;
    }

    cridat_file& file = it->second;
    if (file.archive) {
        stbuf.st_mode = 0755 | S_IFDIR;
    } else {
        stbuf.st_mode = 0444 | S_IFREG;
    }

    stbuf.st_size = file.size;
    return 0;
}

int cridat_archive::open(std::string_view name, int flags) {
    (void) flags;
    auto it = _files.find(name);
    if (it == _files.end()) {
        return -ENOENT;
    }

    /* Can't open archive */
    if (it->second.archive) {
        return -EIO;
    }

    return 0;
}

int cridat_archive::read(std::string_view name, std::span<std::byte> buf, off_t offset) {
    auto it = _files.find(name);
    if (it == _files.end()) {
        return -ENOENT;
    }

    /* Can't open archive */
    if (it->second.archive) {
        return -EIO;
    }

    it->second.fs->seek(offset);
    return it->second.fs->read(buf);
}
