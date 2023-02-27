// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIPAK_HPP
#define CRIPAK_HPP

#include "archive/archive.hpp"

#include <memory>
#include <filesystem>
#include <map>

#include "util/cripak_utf.hpp"

class cripak_archive : public archive {
    std::unique_ptr<utf_table> _cpk;
    
    struct cripak_file {
        std::string name;

        uint32_t file_size;
        uint32_t extract_size;

        uint64_t file_offset;
        uint32_t id;

        std::string user_string;
        uint32_t crc;

        uint64_t update_datetime;
        std::string local_dir;
    };

    struct directory {
        std::vector<cripak_file> files;
        std::map<std::string, directory> dirs;
    };

    directory _root;

    public:
    explicit cripak_archive(std::string_view name, file_stream& cripak_fs);

    void contents(fill_func filler) override;

    [[nodiscard]] bool contains_archive(const std::filesystem::path& path) override;
    [[nodiscard]] archive& get_archive(const std::filesystem::path& path) override;
    [[nodiscard]] int stat(const std::filesystem::path& path, struct stat& stbuf) override;
};

#endif /* CRIPAK_HPP */
