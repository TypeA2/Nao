#pragma once

#include "binary_stream.h"

#include <portaudio.h>

enum sample_type : uintmax_t {
    SAMPLE_NONE    = 0b00,
    SAMPLE_INT16   = 0b01,
    SAMPLE_FLOAT32 = 0b10
};

template <sample_type type>
struct underlying_type { };

template <>
struct underlying_type<SAMPLE_INT16> { using type = int16_t; };

template <>
struct underlying_type<SAMPLE_FLOAT32> { using type = float; };

template <sample_type type>
using underlying_type_t = typename underlying_type<type>::type;

enum pcm_state : int64_t {
    PCM_DONE = 0,
    PCM_ERR = -1
};

class pcm_provider {
    public:
    explicit pcm_provider(istream_ptr stream);
    virtual ~pcm_provider() = 0;

    // Retrieve some samples, return 0 for eof, negative for error
    // or positive for read frames (sample_count = frames * channel_count), interleaved
    virtual int64_t get_samples(void*& data, sample_type type) = 0;
    virtual int64_t rate() const = 0;
    virtual int64_t channels() const = 0;

    virtual std::chrono::nanoseconds duration() const = 0;
    virtual std::chrono::nanoseconds pos() const = 0;
    virtual void seek(std::chrono::nanoseconds pos) = 0;

    // Returns a bit field of all available formats
    virtual sample_type types() const = 0;

    // Should return the native-est type available
    virtual sample_type preferred_type() const = 0;

    static size_t sample_size(sample_type type);
    static PaSampleFormat pa_format(sample_type type);

    static void delete_samples(void* data, sample_type type);
    static void convert_samples(void* from, sample_type src_type,
        void*& to, sample_type dest_type, int64_t frames, int64_t channels);

    protected:
    istream_ptr stream;
};

using pcm_provider_ptr = std::shared_ptr<pcm_provider>;
