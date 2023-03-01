// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIPAK_HPP
#define CRIPAK_HPP

#include "archive/archive.hpp"

#include <memory>
#include <filesystem>
#include <map>

#include "util/cripak_utf.hpp"
#include "util/file_stream.hpp"

class cripak_archive : public file_archive {
    std::unique_ptr<utf_table> _cpk;

    uint64_t _content_offset;
    
    struct cripak_file {
        std::string name;

        uint32_t file_size;
        uint32_t extract_size;

        uint64_t file_offset;
        uint32_t id;

        std::string user_string = "";
        uint32_t crc;

        uint64_t update_datetime;
        std::string local_dir;

        std::unique_ptr<file_stream> stream = nullptr;
    };

    struct directory : archive {
        std::vector<cripak_file> files;
        std::map<std::string, directory> dirs;

        void contents(fill_func filler) override;

        [[nodiscard]] bool contains_archive(std::string_view name) override;
        [[nodiscard]] archive& get_archive(std::string_view name) override;
        [[nodiscard]] int stat(std::string_view name, struct stat& stbuf) override;
        [[nodiscard]] int open(std::string_view name, int flags) override;
        [[nodiscard]] int read(std::string_view name, std::span<std::byte> buf, off_t offset) override;
    };

    directory _root;

    public:
    explicit cripak_archive(std::string_view name, std::unique_ptr<file_stream> cripak_fs);

    void contents(fill_func filler) override;

    [[nodiscard]] bool contains_archive(std::string_view name) override;
    [[nodiscard]] archive& get_archive(std::string_view name) override;
    [[nodiscard]] int stat(std::string_view name, struct stat& stbuf) override;
    [[nodiscard]] int open(std::string_view name, int flags) override;
    [[nodiscard]] int read(std::string_view name, std::span<std::byte> buf, off_t offset) override;

    private:
    /**
     * @brief Convert a CPK-format 64-bit timestamp to a conventional Unix second-based timestamp
     * 
     * @param cpk_datetime 
     * @return uint64_t 
     */
    [[nodiscard]] static uint64_t convert_datetime(uint64_t cpk_datetime);
};

#endif /* CRIPAK_HPP */
