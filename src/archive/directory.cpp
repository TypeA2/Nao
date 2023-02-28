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
    for (auto& entry : std::filesystem::directory_iterator()) {
        filler(entry.path().filename().c_str(), entry.is_directory() ? file_type::dir : file_type::file);
    }
}

bool directory_archive::contains_archive(std::string_view name) {
    return archive::resolve(name);
}

archive& directory_archive::get_archive(std::string_view name) {
    std::string name_str { name };

    if (_files.contains(name_str)) {
        return *_files.at(name_str);
    }

    auto new_path = _path / name;
    if (is_directory(new_path)) {
        if (archive::resolve(new_path, nullptr, &_files[name_str])) {
            return *_files[name_str];
        }

        throw archive_error("error creating archive for directory");
    } else {
        if (archive::resolve(new_path, std::make_unique<mmapped_file>(new_path), &_files[name_str])) {
            return *_files[name_str];
        }

        throw archive_error("error creating archive for file");
    }
}

int directory_archive::stat(std::string_view name, struct stat& stbuf) {
    spdlog::debug("{}", name);
    auto temp_path = _path / name;
    return ::stat(temp_path.c_str(), &stbuf);
}
