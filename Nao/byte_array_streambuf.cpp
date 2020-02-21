#include "byte_array_streambuf.h"

#include "utils.h"

byte_array_streambuf::byte_array_streambuf(const char* data, size_t size) {
    setg(const_cast<char*>(data), const_cast<char*>(data), const_cast<char*>(data) + size);
}

std::streambuf::pos_type byte_array_streambuf::seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode) {
    switch (dir) {
        case std::ios::beg: setg(eback(), eback() + offset, egptr()); break;
        case std::ios::cur: setg(eback(), gptr() + offset, egptr()); break;
        case std::ios::end: setg(eback(), egptr() + offset, egptr()); break;
        default: break;
    }

    return std::distance(eback(), gptr());
}

std::streambuf::pos_type byte_array_streambuf::seekpos(pos_type pos, std::ios_base::openmode which) {
    return seekoff(pos, std::ios::beg, which);
}

