// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PARTIAL_FILE_HPP
#define PARTIAL_FILE_HPP

#include "util/file_stream.hpp"

class partial_file : public file_stream {
    file_stream& _stream;
    std::streamoff _start;
    std::streamsize _size;

    public:
    explicit partial_file(file_stream& stream, std::streamoff start, std::streamsize size);

    ~partial_file() override = default;

    [[nodiscard]] std::streamsize size() const override;

    void seek(std::streamoff new_pos) override;

    [[nodiscard]] std::streamsize read(std::span<std::byte> dest) override;

    [[nodiscard]] std::byte getb();
};

#endif /* PARTIAL_FILE_HPP */
