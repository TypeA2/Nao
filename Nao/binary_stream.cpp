#include "binary_stream.h"

binary_stream::binary_stream(stream s)
    : file { std::move(s) }  {

}

std::streampos binary_stream::tellg() const {
    return file->tellg();
}

binary_stream& binary_stream::seekg(pos_type pos, seekdir dir) {
    file->seekg(pos, dir);

    return *this;
}

bool binary_stream::eof() const {
    return file->eof();
}

binary_stream& binary_stream::ignore(std::streamsize max, std::istream::int_type delim) {
    file->ignore(max, delim);

    return *this;
}


class binary_stream& binary_stream::rseek(pos_type pos) {
    return seekg(pos, std::ios::cur);
}

binary_stream& binary_stream::read(char* buf, std::streamsize count) {
    file->read(buf, count);

    return *this;
}
