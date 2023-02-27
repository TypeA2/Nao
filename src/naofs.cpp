// SPDX-License-Identifier: LGPL-3.0-or-later

#include "naofs.hpp"

#include <fcntl.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

naofs::naofs(std::string_view source, archive_mode mode)
    : _mode { mode }, _path { std::filesystem::canonical(source) } {
    if (!std::filesystem::exists(_path)) {
        throw std::runtime_error(fmt::format("\"{}\" does not exist!", source));
    }

    if (!std::filesystem::is_regular_file(_path)) {
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

    if (int res = fstat(fd.fd, &_stbuf); res != 0) {
        throw std::system_error(res, std::generic_category(), "fstat");
    }

    _root_file = std::make_unique<mmapped_file>(fd.fd);
    _root = archive::resolve(std::string_view(_path.c_str()), *_root_file);

    if (!_root) {
        throw std::runtime_error(fmt::format("Unknown file {}", _path.c_str()));
    }
}

int naofs::getattr(const std::filesystem::path& path, struct stat& stbuf) {
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

        return cur.stat(path.filename().c_str(), stbuf);
    }

    return 0;
}

int naofs::readdir(const std::filesystem::path& path, off_t offset, fill_dir filler) {
    (void) offset;
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

std::expected<std::reference_wrapper<archive>, int> naofs::get_subarchive(const std::filesystem::path& path) const {
    archive* cur = _root.get();

    if (path == "/") {
        return *cur;
    }

    for (const auto& component : path.lexically_relative("/")) {
        if (!cur->contains_archive(component.c_str())) {
            return std::unexpected(-ENOENT);
        }

        cur = &cur->get_archive(component.c_str());
    }

    return *cur;
}
