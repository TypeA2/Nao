#pragma once

#include "binary_stream.h"

// Common interface for opus/vorbis decoding
class opus_vorbis_decoder {
    public:
    explicit opus_vorbis_decoder(const istream_ptr& stream);
    virtual ~opus_vorbis_decoder() = default;

    bool valid() const;

    std::chrono::nanoseconds duration() const;
    uint32_t rate() const;
    int channels() const;

    virtual std::chrono::nanoseconds pos() const = 0;
    virtual void seek(std::chrono::nanoseconds pos) const = 0;

    // Returns audio frames written
    virtual [[nodiscard]] int64_t read(short* buffer, int64_t frames) const = 0;
    virtual [[nodiscard]] int64_t read_float(float* buffer, int64_t frames) const = 0;

    protected:
    istream_ptr stream;

    bool is_valid { };

    std::chrono::nanoseconds total_duration { };
    uint32_t sample_rate { };
    int channel_count { };
};
