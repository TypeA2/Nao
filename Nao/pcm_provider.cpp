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
    : _data { std::move(other._data) }, _frames { other._frames }, _channels { other._channels }
    , _type { other._type }, _order { other._order } {
    
}

pcm_samples& pcm_samples::operator=(pcm_samples&& other) noexcept {
    _data = std::move(other._data);
    _frames = other._frames;
    _channels = other._channels;
    _type = other._type;
    _order = other._order;

    return *this;
}

pcm_samples::pcm_samples(pcm_state state) : pcm_samples(SAMPLE_NONE, state, 0, CHANNELS_NONE) {
    
}

pcm_samples::pcm_samples(sample_type type, int64_t frames, int64_t channels, channel_order order)
    : _data(sample_size(type) * frames * channels, 0)
    , _frames { frames }, _channels { channels }, _type { type }, _order { order } {
    
}

char* pcm_samples::data() {
    return _data.data();
}

const char* pcm_samples::data() const {
    return _data.data();
}

pcm_samples::operator bool() const {
    return !_data.empty();
}

int64_t pcm_samples::frames() const {
    return _frames;
}

int64_t pcm_samples::channels() const {
    return _channels;
}

sample_type pcm_samples::type() const {
    return _type;
}

channel_order pcm_samples::order() const {
    return _order;
}

int64_t pcm_samples::samples() const {
    return _frames * _channels;
}

size_t pcm_samples::sample_size() const {
    return sample_size(_type);
}

size_t pcm_samples::frame_size() const {
    return sample_size(_type) * _channels;
}

pcm_samples& pcm_samples::scale(float scalar) {
    switch (_type) {
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
    ASSERT(frames <= _frames);

    if (frames == _frames) {
        return *this;
    }

    _frames = frames;
    _data.resize(_frames * _channels * sample_size(_type));

    return *this;
}

pcm_samples& pcm_samples::resize_samples(int64_t samples) {
    ASSERT(samples <= this->samples());

    if (samples == this->samples()) {
        return *this;
    }

    _frames = samples / _channels;
    _data.resize(samples * sample_size(_type));
    return *this;
}

pcm_samples& pcm_samples::resize_bytes(size_t bytes) {
    ASSERT(bytes <= _data.size());

    if (bytes == _data.size()) {
        return *this;
    }

    _frames = bytes / _channels / sample_size(_type);
    _data.resize(bytes);
    return *this;
}


pcm_samples pcm_samples::as(sample_type type) const {
    if (type == _type) {
        return *this;
    }

    pcm_samples res { type, _frames, _channels, _order };

    // Source type
    switch (_type) {
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
    ASSERT(_channels == 6 && channels == 2);
    pcm_samples res { _type, _frames, channels, _order };

    // For 6 channels to 2:
    // L = L + k * C + k * Ls
    // R = R + k * C + k * Rs

    int64_t off_left = 0;
    int64_t off_left_s = 0;
    int64_t off_center = 0;
    int64_t off_right_s = 0;
    int64_t off_right = 0;
    switch (_order) {
        case CHANNELS_VORBIS:
            off_left = 0;
            off_left_s = 3;
            off_center = 1;
            off_right_s = 4;
            off_right = 2;
            break;
        case CHANNELS_WAV:
            off_left = 0;
            off_left_s = 4;
            off_center = 2;
            off_right = 1;
            off_right_s = 5;
            break;

        case CHANNELS_SMPTE:
        case CHANNELS_NONE: ASSERT(false);
    }

    static constexpr float k = 0.7071f;

    size_t res_index = 0;
    switch (_type) {
        case SAMPLE_INT16: {
            const sample_int16_t* src = data<SAMPLE_INT16>();
            sample_int16_t* dest = res.data<SAMPLE_INT16>();

            static constexpr float min = std::numeric_limits<sample_int16_t>::min();
            static constexpr float max = std::numeric_limits<sample_int16_t>::max();

            for (int64_t i = 0; i < samples(); i += _channels) {
                dest[res_index++] = static_cast<sample_int16_t>(std::clamp(
                    static_cast<float>(src[i + off_left]) +
                    (k * static_cast<float>(src[i + off_center])) +
                    (k * static_cast<float>(src[i + off_left_s])), min, max));

                dest[res_index++] = static_cast<sample_int16_t>(std::clamp(
                    static_cast<float>(src[i + off_right]) +
                    (k * static_cast<float>(src[i + off_center])) +
                    (k * static_cast<float>(src[i + off_right_s])), min, max));
            }
            break;
        }

        case SAMPLE_FLOAT32: {
            auto src = data<SAMPLE_FLOAT32>();
            auto dest = res.data<SAMPLE_FLOAT32>();

            for (int64_t i = 0; i < samples(); i += _channels) {
                dest[res_index++] = std::clamp(
                    src[i + off_left] + k * src[i + off_center] + k * src[i + off_left_s], -1.f, 1.f);

                dest[res_index++] = std::clamp(
                    src[i + off_right] + k * src[i + off_center] + k * src[i + off_right_s], -1.f, 1.f);
            }
            break;
        }

        case SAMPLE_NONE: ASSERT(false);
    }

    return res;
}

pcm_provider::pcm_provider(const istream_ptr& stream) : stream(stream) {
    
}
