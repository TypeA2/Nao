#include "binary_stream.h"

#include "utils.h"

#include <fstream>

#include "frameworks.h"

#include "byte_array_streambuf.h"

#include <nao/strings.h>

binary_istream::binary_istream(const std::string& path)
    : file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}

binary_istream::binary_istream(const std::filesystem::path& path)
    : file { std::make_unique<std::fstream>(path, std::ios::in | std::ios::binary) } {
    
}

binary_istream::binary_istream(const std::shared_ptr<std::istream>& file) : file { file } {

}

binary_istream::binary_istream(binary_istream&& other) noexcept
    : file { std::move(other.file) }, streambuf { other.streambuf.release() } {
    other.file = nullptr;
}

binary_istream::binary_istream(int resource, const std::string& type) {
    HRSRC handle = FindResourceW(nullptr, MAKEINTRESOURCEW(resource), nao::to_utf16(type).c_str());
    HGLOBAL res = LoadResource(nullptr, handle);
    streambuf = std::make_unique<byte_array_streambuf>(static_cast<char*>(LockResource(res)),
        SizeofResource(nullptr, handle));

    file = std::make_unique<std::istream>(streambuf.get());
}

binary_istream::binary_istream(std::unique_ptr<std::streambuf> buf)
    : file { std::make_shared<std::istream>(buf.get()) }, streambuf { std::move(buf) } {

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
    ASSERT(!_m_bitwise);

    std::unique_lock lock(mutex);
    file->read(buf, count);

    return *this;
}

void binary_istream::set_bitwise(bool bitwise) {
    if (!bitwise) {
        _m_bit_buffer_size = 0;
        _m_bit_buffer = 0;
    }
    _m_bitwise = bitwise;
}

bool binary_istream::get_bit() {
    ASSERT(_m_bitwise);

    if (_m_bit_buffer_size == 0) {
        if (file->eof()) {
            throw std::runtime_error("no more data");
        }

        if (!file->good()) {
            file->clear();
        }

        _m_bit_buffer = file->get();
        _m_bit_buffer_size = 8;
    }

    --_m_bit_buffer_size;

    return ((_m_bit_buffer & (0b10000000 >> _m_bit_buffer_size)) != 0);
}

uintmax_t binary_istream::read_bits(size_t bits) {
    uintmax_t val = 0;

    for (size_t i = 0; i < bits; ++i) {
        if (get_bit()) {
            val |= (1ui64 << i);
        }
    }

    return val;
}

binary_ostream& binary_ostream::write_bits(uintmax_t val, size_t bits) {
    ASSERT(_m_bitwise);
    if (bits >= (bit_buffer_limit - _m_bit_buffer_size)) {
        size_t bits_to_put = bit_buffer_limit - _m_bit_buffer_size;
        _m_bit_buffer |= (static_cast<uint64_t>(val & ((1ui64 << bits_to_put) - 1)) << _m_bit_buffer_size);

        val >>= bits_to_put;
        _m_bit_buffer_size += bits_to_put;

        flush_bits();

        if (bits_to_put != bits) {
            _m_bit_buffer |= static_cast<uint64_t>(val);
            _m_bit_buffer_size += (bits - bits_to_put);
        }


        return *this;
    }

    // There's enough place to emplace the entire value
    _m_bit_buffer |= (static_cast<uint64_t>(val) << _m_bit_buffer_size);
    _m_bit_buffer_size += bits;

    return *this;
}

binary_ostream::binary_ostream(const std::filesystem::path& path)
    : file { std::make_unique<std::fstream>(path, std::ios::out | std::ios::binary) } {

}

binary_ostream::binary_ostream(const std::shared_ptr<std::ostream>& stream)
    : file { stream } {
    
}

binary_ostream::binary_ostream(binary_ostream&& other) noexcept
    : file { std::move(other.file) } {
    other.file = nullptr;
}

binary_ostream::~binary_ostream() {
    if (_m_bitwise) {
        flush_bits();
    }
}


binary_ostream& binary_ostream::write(const char* buf, std::streamsize size) {
    ASSERT(!_m_bitwise);
    file->write(buf, size);

    return *this;
}

binary_ostream::pos_type binary_ostream::tellp() const {
    return file->tellp();
}

void binary_ostream::set_bitwise(bool bitwise) {
    if (!bitwise) {
        flush_bits();
    }

    _m_bitwise = bitwise;
}

bool binary_ostream::bitwise() const {
    return _m_bitwise;
}

binary_ostream& binary_ostream::put_bit(bool value) {
    ASSERT(_m_bitwise);

    if (value) {
        _m_bit_buffer |= (1ui64 << _m_bit_buffer_size);
    }

    ++_m_bit_buffer_size;

    if (_m_bit_buffer_size == bit_buffer_limit) {
        flush_bits();
    }

    return *this;
}

binary_ostream& binary_ostream::flush_bits() {
    if (_m_bit_buffer_size > 0) {
        size_t bytes = (_m_bit_buffer_size + (CHAR_BIT - 1)) / CHAR_BIT;
        file->write(reinterpret_cast<char*>(&_m_bit_buffer), bytes);
    }

    _m_bit_buffer = 0;
    _m_bit_buffer_size = 0;

    return *this;
}

binary_iostream::binary_iostream(const std::shared_ptr<std::iostream>& stream)
    : binary_istream(stream), binary_ostream(stream), _m_stream { stream } {
    
}

std::iostream* binary_iostream::stream() const {
    return _m_stream.get();
}
