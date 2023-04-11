#include "archive/directory.hpp"

#include <fcntl.h>

#include <spdlog/spdlog.h>

#include "util/exceptions.hpp"
#include "util/mmapped_file.hpp"
#include "util/fstream_file.hpp"

directory_archive::directory_archive(const std::filesystem::path& path)
    : _path { std::filesystem::canonical(path) } {
    
    spdlog::info("Opening directory: {}", path.string());

    if (!is_directory(_path)) {
        throw archive_error("\"{}\" is not a directory", _path.string());
    }

    for (auto& entry : std::filesystem::directory_iterator(_path)) {
        const auto& path = entry.path();

        /* Disk directories are resolved too */
        if (entry.is_directory()) {
            _archives.insert({ path.filename(), archive::get_archive(path, nullptr) });
        } else {
            /* Non-directories at least get a file stream */
            auto fs = std::make_unique<fstream_file>(path);

            if (archive::is_archive(path, fs.get())) {
                _archives.insert({ path.filename(), archive::get_archive(path, std::move(fs)) });
            } else {
                _files.insert({ path.filename(), std::move(fs) });
            }
        }
    }
}

void directory_archive::contents(fill_func filler) {
    for (const auto& [name, _] : _archives) {
        filler(name, file_type::dir);
    }

    for (const auto& [name, _] : _files) {
        filler(name, file_type::file);
    }
}

bool directory_archive::contains_archive(std::string_view name) {
    return _archives.contains(std::string { name });
}

archive& directory_archive::get_archive(std::string_view name) {
    auto it = _archives.find(std::string { name });

    if (it == _archives.end()) {
        throw archive_error("archive {} does not exist", name);
    }

    return *it->second;
}

int directory_archive::stat(std::string_view name, struct stat& stbuf) {
    auto temp_path = _path / name;

    int res = ::stat(temp_path.c_str(), &stbuf);
    if (res != 0) {
        return res;
    }

    if (_archives.contains(std::string { name })) {
        /* Treat all archives as directories, so overwrite file type field */
        stbuf.st_mode = (stbuf.st_mode & ~S_IFMT) | S_IFDIR;
    }
    
    return 0;
}

int directory_archive::open(std::string_view name, int flags) {
    if ((flags & O_ACCMODE) != O_RDONLY) {
        return -EROFS;
    }

    std::string name_str { name };

    /* Sanity check */
    if (_archives.contains(name_str)) {
        return -EISDIR;
    }

    /* Another sanity check */
    if (!_files.contains(name_str)) {
        return -EEXIST;
    }

    return 0;
}

int directory_archive::read(std::string_view name, std::span<std::byte> buf, off_t offset) {
    std::string name_str { name };

    if (!_files.contains(name_str)) {
        return -EBADF;
    }

    /* Read from open file */
    file_stream& fs = *_files[name_str];

    fs.seek(offset);

    return fs.read(buf);
}
