// SPDX-License-Identifier: LGPL-3.0-or-later

#include "naofs.hpp"

#include <fcntl.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

naofs::naofs(const std::filesystem::path& source, archive_mode mode)
    : _mode { mode }, _path { std::filesystem::canonical(source) } {
    if (!std::filesystem::exists(_path)) {
        throw std::runtime_error(fmt::format("\"{}\" does not exist!", source.string()));
    }

    if (int res = stat(_path.c_str(), &_stbuf); res != 0) {
        throw std::system_error(res, std::generic_category(), "fstat");
    }

    if (is_directory(_path)) {
        if (!archive::resolve(_path, nullptr, &_root)) {
            throw std::runtime_error(fmt::format("Unknown directory \"{}\"", _path.c_str()));
        }
    } else {
        if (!archive::resolve(_path, std::make_unique<mmapped_file>(_path), &_root)) {
            throw std::runtime_error(fmt::format("Unknown file \"{}\"", _path.c_str()));
        }
    }

    
}

int naofs::getattr(const std::filesystem::path& path, struct stat& stbuf) {
    spdlog::trace("getattr \"{}\"", path.string());

    /* These always match the file's owner */
    stbuf.st_uid  = _stbuf.st_uid;
    stbuf.st_gid  = _stbuf.st_gid;

    /* Default values */
    stbuf.st_atim = _stbuf.st_atim;
    stbuf.st_mtim = _stbuf.st_mtim;
    stbuf.st_ctim = _stbuf.st_ctim;

    if (path == "/") {
        /* Root, read-only directory */
        stbuf.st_mode = 0755 | S_IFDIR;

    } else {
        /* Querry corresponding archive */
        auto expected = get_subarchive(path.parent_path());
        if (!expected) {
            return expected.error();
        }

        archive& cur = expected.value();

        return cur.stat(path.filename().string(), stbuf);
    }

    return 0;
}

int naofs::readdir(const std::filesystem::path& path, off_t offset, fill_dir filler) {
    (void) offset;
    spdlog::trace("readdir \"{}\"", path.string());

    auto expected = get_subarchive(path);
    if (!expected) {
        return expected.error();
    }

    archive& cur = expected.value();

    /* These always exist */
    filler(".", nullptr, 0);
    filler("..", nullptr, 0);

    cur.contents([&](std::string_view name, archive::file_type type) {
        struct stat st;
        st.st_mode = (type == archive::file_type::dir) ? S_IFDIR : S_IFREG;

        filler(std::string(name), &st, 0);
    });

    return 0;
}

int naofs::open(const std::filesystem::path& path, int flags) {
    spdlog::trace("open \"{}\"", path.string());

    if ((flags & O_ACCMODE) != O_RDONLY) {
        return -EROFS;
    }

    auto expected = get_subarchive(path.parent_path());
    if (!expected) {
        return expected.error();
    }

    archive& cur = expected.value();

    return cur.open(path.filename().string(), flags);
}

int naofs::read(const std::filesystem::path& path, std::span<std::byte> buf, off_t offset) {
    spdlog::trace("read \"{}\"", path.string());

    auto expected = get_subarchive(path.parent_path());
    if (!expected) {
        return expected.error();
    }

    archive& cur = expected.value();

    return cur.read(path.filename().string(), buf, offset);
}

std::expected<std::reference_wrapper<archive>, int> naofs::get_subarchive(const std::filesystem::path& path) const {
    archive* cur = _root.get();

    if (path == "/") {
        return *cur;
    }

    for (const auto& component : path.lexically_relative("/")) {
        if (!cur->contains_archive(component.string())) {
            return std::unexpected(-ENOENT);
        }

        cur = &cur->get_archive(component.string());
    }

    return *cur;
}
