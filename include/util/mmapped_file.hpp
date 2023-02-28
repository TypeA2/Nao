// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MMAPPED_FILE_HPP
#define MMAPPED_FILE_HPP

#include "util/file_stream.hpp"

#include <filesystem>

class mmapped_file : public file_stream {
    std::streamsize _size;
    std::byte* _addr;

    public:
    explicit mmapped_file(int fd);
    explicit mmapped_file(const std::filesystem::path& path);

    ~mmapped_file() override;

    [[nodiscard]] std::streamsize size() const override;

    void seek(std::streamoff new_pos) override;

    [[nodiscard]] std::streamsize read(std::span<std::byte> dest) override;

    [[nodiscard]] std::byte getb();
};

#endif /* MMAPPED_FILE_HPP */
