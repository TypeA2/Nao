// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MMAPPED_FILE_H
#define MMAPPED_FILE_H

#include "util/file_stream.hpp"

class mmapped_file : public file_stream {
    size_t _size;
    std::byte* _addr;

    public:
    explicit mmapped_file(int fd);
    ~mmapped_file() override;

    [[nodiscard]] ssize_t read(std::span<std::byte> dest) override;
};

#endif /* MMAPPED_FILE_H */
