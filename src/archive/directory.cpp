#include "archive/directory.hpp"

#include <spdlog/spdlog.h>

#include "util/exceptions.hpp"
#include "util/mmapped_file.hpp"

directory_archive::directory_archive(const std::filesystem::path& path)
    : _path { std::filesystem::canonical(path) } {
    
    spdlog::info("Opening directory: {}", path.string());

    if (!is_directory(_path)) {
        throw archive_error("\"{}\" is not a directory", _path.string());
    }
}

void directory_archive::contents(fill_func filler) {
    for (auto& entry : std::filesystem::directory_iterator(_path)) {
        filler(entry.path().filename().string(), entry.is_directory() ? file_type::dir : file_type::file);
    }
}

bool directory_archive::contains_archive(std::string_view name) {
    return archive::resolve(_path / name);
}

archive& directory_archive::get_archive(std::string_view name) {
    std::string name_str { name };

    if (_archives.contains(name_str)) {
        return *_archives.at(name_str);
    }

    auto new_path = _path / name;
    if (is_directory(new_path)) {
        if (archive::resolve(new_path, nullptr, &_archives[name_str])) {
            return *_archives[name_str];
        }

        throw archive_error("error creating archive for directory");
    } else {
        if (archive::resolve(new_path, std::make_unique<mmapped_file>(new_path), &_archives[name_str])) {
            return *_archives[name_str];
        }

        throw archive_error("error creating archive for file");
    }
}

int directory_archive::stat(std::string_view name, struct stat& stbuf) {
    auto temp_path = _path / name;
    return ::stat(temp_path.c_str(), &stbuf);
}

int directory_archive::open(std::string_view name, int flags) {
    std::string name_str { name };

    /* Sanity check */
    if (_archives.contains(name_str)) {
        return -EISDIR;
    }

    /* If the file is already open, do nothing */
    if (_files.contains(name_str)) {
        return 0;
    }

    /* Else open the file and store the handle */
    _files[name_str] = std::make_unique<mmapped_file>(_path / name);
    if (!_files.at(name_str)) {
        return -EIO;
    }

    return 0;
}

int directory_archive::read(std::string_view name, std::span<std::byte> buf, off_t offset) {
    std::string name_str { name };

    if (!_files.contains(name_str)) {
        return -EBADF;
    }

    file_stream& fs = *_files[name_str];

    fs.seek(offset);

    return fs.read(buf);
}
