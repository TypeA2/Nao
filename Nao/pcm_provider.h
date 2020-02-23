#pragma once

#include "binary_stream.h"

#include <portaudio.h>

enum sample_type : uintmax_t {
    SAMPLE_NONE    = 0b00,
    SAMPLE_INT16   = 0b01,
    SAMPLE_FLOAT32 = 0b10
};

inline sample_type operator|(sample_type left, sample_type right) {
    return static_cast<sample_type>(static_cast<uintmax_t>(left) | static_cast<uintmax_t>(right));
}

using sample_int16_t = int16_t;
using sample_float32_t = float;

template <sample_type> struct underlying_type                 {                                };
template <>            struct underlying_type<SAMPLE_INT16>   { using type = sample_int16_t;   };
template <>            struct underlying_type<SAMPLE_FLOAT32> { using type = sample_float32_t; };

template <sample_type type>
using underlying_type_t = typename underlying_type<type>::type;

enum pcm_state : int64_t {
    PCM_DONE = 0,
    PCM_ERR = -1
};

enum channel_order {
    CHANNELS_NONE,

    /**
     * 1: mono
     * 2: left, right
     * 3: left, center, right
     * 4: front left, front right, rear left, rear right
     * 5: front left, center, front right, rear left, rear right
     * 6: front left, center, front right, rear left, rear right, LFE
     * 7: front left, center, front right, side left, side right, rear center, LFE
     * 8: front left, center, front right, side left, side right, rear left, rear right, LFE
     */
    CHANNELS_VORBIS,

    /**
     * front left, front right, front center, LFE, back left, back right, front left of center, front right of center
     */
    CHANNELS_WAV,

    /**
     * left, center, right, rear left, rear right, lfe
     */
    CHANNELS_SMPTE
};

// Encapsulates samples
class pcm_samples final {
    public:
    // Some type checks to make sure our vector-based packing works properly
    static_assert(sizeof(sample_int16_t) == alignof(sample_int16_t));
    static_assert(sizeof(sample_float32_t) == alignof(sample_float32_t));
 
    static pcm_samples error(int64_t code);

    static size_t sample_size(sample_type type);
    static PaSampleFormat pa_format(sample_type type);

    pcm_samples() = delete;
    pcm_samples(pcm_samples&& other) noexcept;
    pcm_samples& operator=(pcm_samples&& other) noexcept;

    pcm_samples(pcm_state state);

    pcm_samples(sample_type type, int64_t frames, int64_t channels, channel_order order);

    template <sample_type type>
    underlying_type_t<type>* data() {
        return reinterpret_cast<underlying_type_t<type>*>(_m_data.data());
    }

    template <sample_type type>
    const underlying_type_t<type>* data() const {
        return reinterpret_cast<const underlying_type_t<type>*>(_m_data.data());
    }

    char* data();
    const char* data() const;

    operator bool() const;

    int64_t frames() const;
    int64_t channels() const;
    sample_type type() const;
    channel_order order() const;

    int64_t samples() const;
    size_t sample_size() const;
    size_t frame_size() const;

    // Scale all values by a scalar
    pcm_samples& scale(float scalar);

    // Resize the value array
    pcm_samples& resize(int64_t frames);

    // Copy and convert to a different sample type
    pcm_samples as(sample_type type) const;

    // Downmix to the specified number of channels
    pcm_samples downmix(int64_t channels) const;

    private:
    pcm_samples(const pcm_samples&) = default;
    pcm_samples& operator=(const pcm_samples& other) = default;

    std::vector<char> _m_data;
    int64_t _m_frames;
    int64_t _m_channels;
    sample_type _m_type;
    channel_order _m_order;
};

class pcm_provider {
    public:
    explicit pcm_provider(const istream_ptr& stream);
    virtual ~pcm_provider() = 0;

    // Retrieve some samples, return 0 for eof, negative for error
    // or positive for read frames (sample_count = frames * channel_count), interleaved
    virtual pcm_samples get_samples(sample_type type) = 0;
    virtual int64_t rate() = 0;
    virtual int64_t channels() = 0;
    virtual channel_order order() = 0;
    virtual std::string name() = 0;

    virtual std::chrono::nanoseconds duration() = 0;
    virtual std::chrono::nanoseconds pos() = 0;
    virtual void seek(std::chrono::nanoseconds pos) = 0;

    // Returns a bit field of all available formats
    virtual sample_type types() = 0;

    // Should return the native-est type available
    virtual sample_type preferred_type() = 0;

    protected:
    istream_ptr stream;
};

using pcm_provider_ptr = std::shared_ptr<pcm_provider>;
