#include "binary_stream.h"

#include <fstream>

binary_stream::binary_stream(const std::string& path, std::ios::openmode mode)
    : file { std::make_unique<std::fstream>(path, mode) }  {
    
}


binary_stream::binary_stream(binary_stream&& other) noexcept
    : file { std::move(other.file) }  {
    other.file = nullptr;
}

std::streampos binary_stream::tellg() const {
    return file->tellg();
}

class binary_stream& binary_stream::seekg(pos_type pos) {
    file->seekg(pos);

    return *this;
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

bool binary_stream::good() const {
    return file->good();
}

class binary_stream& binary_stream::rseek(pos_type pos) {
    return seekg(pos, std::ios::cur);
}

binary_stream& binary_stream::read(char* buf, std::streamsize count) {
    file->read(buf, count);

    return *this;
}
