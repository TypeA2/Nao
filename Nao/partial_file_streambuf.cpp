#include "partial_file_streambuf.h"

partial_file_streambuf::partial_file_streambuf(const istream_ptr& stream, std::streamoff start, std::streamsize size)
    : _stream { stream }, _start { start }, _size { size } {
    setg(_buf, _buf + buf_size, _buf + buf_size);
}

partial_file_streambuf::int_type partial_file_streambuf::underflow() {
    auto cur = _cur();

    if (cur >= _size) {
        return traits_type::eof();
    }

    _stream->seekg(_start + cur);
    
    // How many bytes to read
    auto count = std::min<std::streamsize>(_size - cur, buf_size);

    // Read into buffer
    _stream->read(_buf, count);

    // Advance position
    _buf_pos = cur;

    // Reset get area
    setg(_buf, _buf, _buf + count);

    _buf_valid = true;

    return traits_type::to_int_type(*gptr());
}


std::streamsize partial_file_streambuf::showmanyc() {
    return _size - _cur();
}

partial_file_streambuf::pos_type partial_file_streambuf::seekoff(off_type offset, std::ios::seekdir dir, std::ios::openmode mode) {
    switch (dir) {
        case std::ios::cur: return seekpos(_cur() + offset, mode);
        case std::ios::beg: return seekpos(offset, mode);
        case std::ios::end: return seekpos(_size + offset, mode);
        default: break;
    }

    return -1;
}

partial_file_streambuf::pos_type partial_file_streambuf::seekpos(pos_type pos, std::ios::openmode) {
    if (pos >= _size) {
        return -1;
    }

    // If the new position is inside the buffer
    if (_buf_valid && pos >= _buf_pos && pos < (_buf_pos + std::distance(eback(), gptr()))) {
        setg(_buf, _buf + (pos - _buf_pos), _buf + buf_size);
        return _buf_pos + (pos - _buf_pos);
    }

    // Not inside buffer
    setg(_buf, _buf + buf_size, _buf + buf_size);
    _buf_valid = false;
    _buf_pos = pos;

    return pos;
}

std::streamoff partial_file_streambuf::_cur() const {
    if (!_buf_valid) {
        return _buf_pos;
    }

    return _buf_pos + std::distance(eback(), gptr());
}
