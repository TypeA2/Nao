#pragma once

#include "binary_stream.h"

#include <bitset>

extern "C" {
#include <libavutil/samplefmt.h>
}

enum class sample_format : uint64_t {
    none     = 0b000000,
    uint8    = 0b000001,
    int16    = 0b000010,
    int32    = 0b000100,
    int64    = 0b001000,
    float32  = 0b010000,
    float64  = 0b100000,

    // Select only the bits that specify the type
    type_mask = 0b111111,

    // Bit signaling planar layout
    planar_flag = 0b1000000,

    uint8p   = uint8   | planar_flag,
    int16p   = int16   | planar_flag,
    int32p   = int32   | planar_flag,
    int64p   = int64   | planar_flag,
    float32p = float32 | planar_flag,
    float64p = float64 | planar_flag,
};

namespace samples {
    size_t sample_size(sample_format fmt);

    AVSampleFormat to_av(sample_format fmt);
    sample_format from_av(AVSampleFormat fmt);

    bool is_planar(sample_format fmt);

    std::string format_name(sample_format fmt);

}

inline sample_format operator|(sample_format left, sample_format right) {
    return static_cast<sample_format>(
        static_cast<std::underlying_type_t<sample_format>>(left) |
        static_cast<std::underlying_type_t<sample_format>>(right));
}

inline sample_format operator&(sample_format left, sample_format right) {
    return static_cast<sample_format>(
        static_cast<std::underlying_type_t<sample_format>>(left) &
        static_cast<std::underlying_type_t<sample_format>>(right));
}

// Ensure sizes
static_assert(std::numeric_limits<uint8_t>::digits == 8);
static_assert(std::numeric_limits<int16_t>::digits == 16 - 1); // Sign bit is not counted
static_assert(std::numeric_limits<int32_t>::digits == 32 - 1);
static_assert(std::numeric_limits<int64_t>::digits == 64 - 1);
static_assert(std::numeric_limits<float>::digits == 24); // Mantissa bits
static_assert(std::numeric_limits<double>::digits == 53);

using sample_uint8_t = uint8_t;
using sample_int16_t = int16_t;
using sample_int32_t = int32_t;
using sample_int64_t = int64_t;
using sample_float32_t = float;
using sample_float64_t = double;

// Ensure tight packing
#define CHECK_PACKING(type) static_assert(sizeof(type) == alignof(type))
CHECK_PACKING(sample_uint8_t);
CHECK_PACKING(sample_int16_t);
CHECK_PACKING(sample_int32_t);
CHECK_PACKING(sample_int64_t);
CHECK_PACKING(sample_float32_t);
CHECK_PACKING(sample_float64_t);
#undef CHECK_PACKING

template <sample_format> struct sample_type { };

#define SAMPLE_TYPE_HELPER(id, _type) \
template <> struct sample_type<id> { using type = _type; }; \
template <> struct sample_type<id ## p> { using type = _type; };
SAMPLE_TYPE_HELPER(sample_format::uint8, sample_uint8_t);
SAMPLE_TYPE_HELPER(sample_format::int16, sample_int16_t);
SAMPLE_TYPE_HELPER(sample_format::int32, sample_int32_t);
SAMPLE_TYPE_HELPER(sample_format::int64, sample_int64_t);
SAMPLE_TYPE_HELPER(sample_format::float32, float);
SAMPLE_TYPE_HELPER(sample_format::float64, double);
#undef SAMPLE_TYPE_HELPER
template <sample_format fmt>
using sample_format_t = typename sample_type<fmt>::type;

enum pcm_state : int64_t {
    PCM_DONE = 0,
    PCM_ERR = -1
};

// Encapsulates audio samples
class pcm_samples final {
    sample_format _type = sample_format::none;
    uint64_t _frames = 0;
    uint8_t _channels;

    // avutil channel layout
    uint64_t _channel_layout = 0;

    std::vector<char> _data { };
    public:
    pcm_samples() = default;
    pcm_samples(sample_format type, uint64_t frames, uint8_t channels, uint64_t channel_layout);

    int64_t frames() const;
    uint8_t channels() const;
    int64_t samples() const;
    size_t bytes() const;

    char* data();
    const char* data() const;

    operator bool() const;

    template <sample_format type>
    sample_format_t<type>* data() {
        return reinterpret_cast<sample_format_t<type>*>(_data.data());
    }

    template <sample_format type>
    const sample_format_t<type>* data() const {
        return reinterpret_cast<const sample_format_t<type>*>(_data.data());
    }

    template <concepts::iterator<char> InputIt>
    size_t fill(InputIt begin, InputIt end) {
        if (std::distance(begin, end) > _data.size()) {
            throw std::out_of_range("tried to fill past end");
        }

        return std::distance(_data.begin(),
            std::copy(begin, end, _data.begin()));
    }

    template <concepts::iterator<char> InputIt>
    size_t fill_n(InputIt begin, size_t count) {
        if (count > _data.size()) {
            throw std::out_of_range("tried to fill past end");
        }

        return std::distance(_data.begin(),
            std::copy_n(begin, count, _data.begin()));
    }
};

class pcm_decode_exception : public std::runtime_error {
    public:
    using std::runtime_error::runtime_error;
};

class pcm_provider {
    protected:
    istream_ptr stream;

    public:
    explicit pcm_provider(istream_ptr stream);
    virtual ~pcm_provider() = default;

    virtual pcm_samples get_samples() = 0;
    virtual int64_t rate() = 0;
    virtual int64_t channels() = 0;
    virtual std::string name() = 0;

    virtual std::chrono::nanoseconds duration() = 0;
    virtual std::chrono::nanoseconds pos() = 0;
    virtual void seek(std::chrono::nanoseconds pos) = 0;

    virtual sample_format format() = 0;
};

using pcm_provider_ptr = std::shared_ptr<pcm_provider>;
