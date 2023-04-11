// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/file_stream.hpp"

file_stream::file_stream()
    : pos { 0 } {

}

std::streamoff file_stream::tell() const {
    return pos;
}

char file_stream::getc() {
    return static_cast<char>(getb());
}

uint8_t file_stream::read_u8() {
    return static_cast<uint8_t>(getb());
}

uint16_t file_stream::read_u16le() {
    return read_pod<uint16_t, std::endian::little>();
}

uint32_t file_stream::read_u32le() {
    return read_pod<uint32_t, std::endian::little>();
}

uint64_t file_stream::read_u64le() {
    return read_pod<uint64_t, std::endian::little>();
}

uint16_t file_stream::read_u16be() {
    return read_pod<uint16_t, std::endian::big>();
}

uint32_t file_stream::read_u32be() {
    return read_pod<uint32_t, std::endian::big>();
}

uint64_t file_stream::read_u64be() {
    return read_pod<uint64_t, std::endian::big>();
}

uint8_t file_stream::read_s8() {
    return static_cast<int8_t>(getb());
}

uint16_t file_stream::read_s16le() {
    return read_pod<int16_t, std::endian::little>();
}

uint32_t file_stream::read_s32le() {
    return read_pod<int32_t, std::endian::little>();
}

uint64_t file_stream::read_s64le() {
    return read_pod<int64_t, std::endian::little>();
}

uint16_t file_stream::read_s16be() {
    return read_pod<int16_t, std::endian::big>();
}

uint32_t file_stream::read_s32be() {
    return read_pod<int32_t, std::endian::big>();
}

uint64_t file_stream::read_s64be() {
    return read_pod<int64_t, std::endian::big>();
}

float file_stream::read_f32le() {
    return read_pod<float, std::endian::little>();
}

float file_stream::read_f32be() {
    return read_pod<float, std::endian::big>();
}

double file_stream::read_f64le() {
    return read_pod<double, std::endian::little>();
}

double file_stream::read_f64be() {
    return read_pod<double, std::endian::big>();
}

std::string file_stream::read_cstring(char term) {
    return read_ncstring(size_t(-1), term);
}

std::string file_stream::read_ncstring(size_t n, char term) {
    std::string res;
    size_t i = 0;

    char ch;
    while ((i < n) && ((ch = getc()) != term)) {
        res.push_back(ch);
        ++i;
    }

    return res;
}
