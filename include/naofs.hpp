// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NAOFS_H
#define NAOFS_H

#include <string>
#include <string_view>
#include <functional>

#include <filesystem>
namespace fs = std::filesystem;

#include <sys/stat.h>

#include "archive/archive.hpp"
#include "util/mmapped_file.hpp"

enum class archive_mode {
    all, archive_only
};

class naofs {
    archive_mode _mode;
    fs::path _path;

    std::unique_ptr<mmapped_file> _root_file;
    std::unique_ptr<archive> _root;

    public:
    explicit naofs(std::string_view source, archive_mode mode);

    /**
     * @brief Get file attributes for a specific path
     * 
     * @param path 
     * @param stbuf 
     * @return int Error code or 0
     */
    [[nodiscard]] int getattr(std::string_view path, struct stat& stbuf);

    /**
     * Arguments: path, statbuf, index in file list
     * Return value: whether to continue filling the buffer
     */
    using fill_dir = std::function<bool(std::string, struct stat*, off_t)>;

    /**
     * @brief Read directory contents
     * 
     * @param path 
     * @param offset Index at which to start reading contents
     * @param filler Function to call for every element
     * @return int Error code or 0
     */
    [[nodiscard]] int readdir(std::string_view path, off_t offset, fill_dir filler);
};

#endif /* NAOFS_H */
