#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct riff_header {
    char header[4];
    uint32_t size;
};

struct wave_header {
    riff_header riff;
    char wave[4];
};

struct fmt_chunk {
    riff_header riff;
    uint16_t format;
    uint16_t channels;
    uint32_t rate;
    uint32_t byte_rate;
    uint16_t align;
    uint16_t bits;
};

#pragma pack(pop)
