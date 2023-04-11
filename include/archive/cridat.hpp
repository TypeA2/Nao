// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRIDAT_HPP
#define CRIDAT_HPP

#include "archive/archive.hpp"

#include <map>

class cridat_archive : public file_archive {
    struct cridat_file {
        uint32_t size;
        uint32_t offset;

        std::unique_ptr<file_stream> fs = nullptr;
        std::unique_ptr<::archive> archive = nullptr;
    };

    /* Transparent comparison: https://stackoverflow.com/a/35525806/8662472 */
    std::map<std::string, cridat_file, std::less<>> _files;

    public:
    explicit cridat_archive(std::string_view name, std::unique_ptr<file_stream> cridat_fs);

    void contents(fill_func filler) override;

    [[nodiscard]] bool contains_archive(std::string_view name) override;
    [[nodiscard]] archive& get_archive(std::string_view name) override;
    [[nodiscard]] int stat(std::string_view name, struct stat& stbuf) override;
    [[nodiscard]] int open(std::string_view name, int flags) override;
    [[nodiscard]] int read(std::string_view name, std::span<std::byte> buf, off_t offset) override;
};

#endif /* CRIDAT_HPP */
