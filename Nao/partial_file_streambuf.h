#pragma once

#include "binary_stream.h"

class partial_file_streambuf : public std::streambuf {
    static constexpr size_t buf_size = 4096;

    istream_ptr _stream;

    std::streamoff _start;
    std::streamsize _size;
    char _buf[buf_size] { };

    std::streamoff _buf_pos = 0;

    bool _buf_valid = false;

    public:
    partial_file_streambuf(const istream_ptr& stream, std::streamoff start, std::streamsize size);

    protected:
    int_type underflow() override;
    std::streamsize showmanyc() override;
    pos_type seekoff(off_type offset, std::ios::seekdir dir, std::ios::openmode mode) override;
    pos_type seekpos(pos_type pos, std::ios::openmode mode) override;

    std::streamoff _cur() const;
};
