#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct riff_header {
    char header[4];
    uint32_t size;
};

struct wave_chunk {
    char wave[4];
};

struct fmt_chunk {
    uint16_t format;
    uint16_t channels;
    uint32_t rate;
    uint32_t byte_rate;
    uint16_t align;
    uint16_t bits;
};

struct fmt_chunk_extensible {
    uint16_t extra_size;
    uint16_t valid_bits;
    uint32_t channel_mask;
};

#pragma pack(pop)
