// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NAOFS_HPP
#define NAOFS_HPP

#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <expected>

#include <sys/stat.h>

#include "archive/archive.hpp"
#include "util/mmapped_file.hpp"

enum class archive_mode {
    all, archive_only
};

class naofs {
    archive_mode _mode;
    std::filesystem::path _path;
    struct stat _stbuf;

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
    [[nodiscard]] int getattr(const std::filesystem::path& path, struct stat& stbuf);

    /**
     * Arguments: path, statbuf, index in file list
     * Return value: whether to continue filling the buffer
     * 
     * Offset and return value are unused if the passed offset is always 0
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
    [[nodiscard]] int readdir(const std::filesystem::path& path, off_t offset, fill_dir filler);

    private:
    std::expected<std::reference_wrapper<archive>, int> get_subarchive(const std::filesystem::path& path) const;
};

#endif /* NAOFS_HPP */
