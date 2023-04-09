// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/partial_file.hpp"

partial_file::partial_file(file_stream& stream, std::streamoff start, std::streamsize size)
    : _stream { stream }, _start { start }, _size { size } { }

std::streamsize partial_file::size() const {
    return _size;
}

void partial_file::seek(std::streamoff new_pos) {
    /* Assume shared access to underlying stream, so seek for every read */
    pos = new_pos;
}

std::streamsize partial_file::read(std::span<std::byte> dest) {
    /* Don't read past end of file */
    if (pos + dest.size() > _size) {
        dest = std::span(dest.data(), _size - pos);
    }

    /* Always seek to position, we can't be sure of exclusive access */
    _stream.seek(_start + pos);
    auto read = _stream.read(dest);

    pos += read;
    return read;
}

std::byte partial_file::getb() {
    /* Check EOF */
    if (pos == _size) {
        throw io_error("eof");
    }

    /* Always seek */
    _stream.seek(_start + pos);
    auto byte = _stream.getb();

    pos += 1;
    return byte;
}
