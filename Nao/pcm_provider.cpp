#include "pcm_provider.h"
#include "utils.h"

pcm_samples pcm_samples::error(int64_t code) {
    return pcm_samples { SAMPLE_NONE, code, 0, CHANNELS_NONE };
}

size_t pcm_samples::sample_size(sample_type type) {
    switch (type) {
        case SAMPLE_INT16:   return sizeof(sample_int16_t);
        case SAMPLE_FLOAT32: return sizeof(sample_float32_t);
        default:             return 0;
    }
}

PaSampleFormat pcm_samples::pa_format(sample_type type) {
    switch (type) {
        case SAMPLE_INT16:   return paInt16;
        case SAMPLE_FLOAT32: return paFloat32;
        default:             return 0;
    }
}

pcm_samples::pcm_samples(pcm_samples&& other) noexcept
    : _m_data { std::move(other._m_data) }, _m_frames { other._m_frames }, _m_channels { other._m_channels }
    , _m_type { other._m_type }, _m_order { other._m_order } {
    
}

pcm_samples& pcm_samples::operator=(pcm_samples&& other) noexcept {
    _m_data = std::move(other._m_data);
    _m_frames = other._m_frames;
    _m_channels = other._m_channels;
    _m_type = other._m_type;
    _m_order = other._m_order;

    return *this;
}

pcm_samples::pcm_samples(pcm_state state) : pcm_samples(SAMPLE_NONE, state, 0, CHANNELS_NONE) {
    
}

pcm_samples::pcm_samples(sample_type type, int64_t frames, int64_t channels, channel_order order)
    : _m_data(sample_size(type) * frames * channels, 0)
    , _m_frames { frames }, _m_channels { channels }, _m_type { type }, _m_order { order } {
    
}

char* pcm_samples::data() {
    return _m_data.data();
}

const char* pcm_samples::data() const {
    return _m_data.data();
}

pcm_samples::operator bool() const {
    return !_m_data.empty();
}

int64_t pcm_samples::frames() const {
    return _m_frames;
}

int64_t pcm_samples::channels() const {
    return _m_channels;
}

sample_type pcm_samples::type() const {
    return _m_type;
}

channel_order pcm_samples::order() const {
    return _m_order;
}

int64_t pcm_samples::samples() const {
    return _m_frames * _m_channels;
}

size_t pcm_samples::sample_size() const {
    return sample_size(_m_type);
}

size_t pcm_samples::frame_size() const {
    return sample_size(_m_type) * _m_channels;
}

pcm_samples& pcm_samples::scale(float scalar) {
    switch (_m_type) {
        case SAMPLE_INT16: {
            sample_int16_t* src = data<SAMPLE_INT16>();

            for (int64_t i = 0; i < samples(); ++i) {
                src[i] = static_cast<sample_int16_t>(static_cast<float>(src[i]) * scalar);
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            sample_float32_t* src = data<SAMPLE_FLOAT32>();

            for (int64_t i = 0; i < samples(); ++i) {
                src[i] = src[i] * scalar, 0.f, 1.f;
            }
            break;
        }

        case SAMPLE_NONE: ASSERT(false);
    }

    return *this;
}

pcm_samples& pcm_samples::resize(int64_t frames) {
    ASSERT(frames <= _m_frames);

    if (frames == _m_frames) {
        return *this;
    }

    _m_frames = frames;
    _m_data.resize(_m_frames * _m_channels * sample_size(_m_type));

    return *this;
}

pcm_samples pcm_samples::as(sample_type type) const {
    if (type == _m_type) {
        return *this;
    }

    pcm_samples res { type, _m_frames, _m_channels, _m_order };

    // Source type
    switch (_m_type) {
        case SAMPLE_INT16: {
            auto src = data<SAMPLE_INT16>();

            // Destination type
            switch (type) {
                case SAMPLE_FLOAT32: {
                    std::transform(src, src + samples(), res.data<SAMPLE_FLOAT32>(),
                        [](sample_int16_t from) -> sample_float32_t {
                            return static_cast<float>(from) / std::numeric_limits<sample_int16_t>::max();
                        });
                    break;
                }

                case SAMPLE_INT16:
                case SAMPLE_NONE: ASSERT(false);
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            auto src = data<SAMPLE_FLOAT32>();

            switch (type) {
                case SAMPLE_INT16: {
                    std::transform(src, src + samples(), res.data<SAMPLE_INT16>(),
                        [](sample_float32_t from) -> sample_int16_t {
                            return static_cast<sample_int16_t>(
                                std::clamp(from, 0.f, 1.f) * std::numeric_limits<sample_int16_t>::max());
                        });
                    break;
                }

                case SAMPLE_FLOAT32:
                case SAMPLE_NONE: ASSERT(false);
            }
        }

        case SAMPLE_NONE: ASSERT(false);
    }

    return res;
}

pcm_samples pcm_samples::downmix(int64_t channels) const {
    ASSERT(_m_channels == 6 && channels == 2);
    ASSERT(_m_order == CHANNELS_VORBIS);

    static constexpr float k = 0.7071f;

    pcm_samples res { _m_type, _m_frames, channels, _m_order };

    size_t res_index = 0;
    switch (_m_type) {
        case SAMPLE_INT16: {
            auto src = data<SAMPLE_INT16>();
            sample_int16_t* dest = res.data<SAMPLE_INT16>();

            for (int64_t i = 0; i < samples(); i += _m_channels) {
                dest[res_index++] = static_cast<sample_int16_t>(std::clamp<float>(src[i] + k * src[i + 1] + k * src[i + 3],
                    std::numeric_limits<sample_int16_t>::min(), std::numeric_limits<sample_int16_t>::max()));

                dest[res_index++] = static_cast<sample_int16_t>(std::clamp<float>(src[i + 2] + k * src[i + 1] + k * src[i + 4],
                    std::numeric_limits<sample_int16_t>::min(), std::numeric_limits<sample_int16_t>::max()));
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            auto src = data<SAMPLE_FLOAT32>();
            auto dest = res.data<SAMPLE_FLOAT32>();

            for (int64_t i = 0; i < samples(); i += _m_channels) {
                dest[res_index++] = std::clamp(src[i] + k * src[i + 1] + k * src[i + 3], 0.f, 1.f);
                dest[res_index++] = std::clamp(src[i + 2] + k * src[i + 1] + k * src[i + 4], 0.f, 1.f);
            }
            break;
        }

        case SAMPLE_NONE: ASSERT(false);
    }

    return res;
}

pcm_provider::pcm_provider(const istream_ptr& stream) : stream(stream) {
    
}

pcm_provider::~pcm_provider() {
    
}
