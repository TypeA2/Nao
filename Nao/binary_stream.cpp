#include "binary_stream.h"

#include <fstream>

binary_istream::binary_istream(const std::string& path)
    : file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}

binary_istream::binary_istream(const std::filesystem::path& path)
    : file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}


binary_istream::binary_istream(binary_istream&& other) noexcept
    : file { std::move(other.file) }  {
    other.file = nullptr;
}

binary_istream::pos_type binary_istream::tellg() const {
    std::unique_lock lock(mutex);

    return file->tellg();
}

class binary_istream& binary_istream::seekg(pos_type pos) {
    std::unique_lock lock(mutex);

    if (file->eof() || !file->good()) {
        file->clear();
    }
    
    file->seekg(pos);

    return *this;
}

binary_istream& binary_istream::seekg(pos_type pos, seekdir dir) {
    std::unique_lock lock(mutex);

    if (file->eof() || !file->good()) {
        file->clear();
    }

    file->seekg(pos, dir);

    return *this;
}

bool binary_istream::eof() const {
    std::unique_lock lock(mutex);
    return file->eof();
}

binary_istream& binary_istream::ignore(std::streamsize max, std::istream::int_type delim) {
    std::unique_lock lock(mutex);
    file->ignore(max, delim);

    return *this;
}

bool binary_istream::good() const {
    std::unique_lock lock(mutex);
    return file->good();
}

std::streamsize binary_istream::gcount() const {
    std::unique_lock lock(mutex);

    return file->gcount();
}

void binary_istream::clear() {
    std::unique_lock lock(mutex);
    file->clear();
}

class binary_istream& binary_istream::rseek(pos_type pos) {
    std::unique_lock lock(mutex);
    file->seekg(pos, std::ios::cur);

    return *this;
}

binary_istream& binary_istream::read(char* buf, std::streamsize count) {
    std::unique_lock lock(mutex);
    file->read(buf, count);

    return *this;
}
