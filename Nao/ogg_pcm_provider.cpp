#include "ogg_pcm_provider.h"

#include <vorbis/vorbisfile.h>
#include <string>

#include "utils.h"

namespace detail {
    static ov_callbacks callbacks {
        .read_func = [](void* ptr, size_t size, size_t nmemb, void* source) -> size_t {
            binary_istream& stream = *static_cast<binary_istream*>(source);

            if (!stream.good() || stream.eof()) {
                return 0;
            }

            return stream.read(ptr, size, nmemb).gcount();
        },

        .seek_func = [](void* source, ogg_int64_t offset, int whence) -> int {
            binary_istream& stream = *static_cast<binary_istream*>(source);

            switch (whence) {
                case SEEK_SET: stream.seekg(offset, std::ios::beg); break;
                case SEEK_CUR: stream.seekg(offset, std::ios::cur); break;
                case SEEK_END: stream.seekg(offset, std::ios::end); break;
                default: return -1;
            }

            return 0;
        },

        .tell_func = [](void* source) -> long {
            return static_cast<long>(static_cast<binary_istream*>(source)->tellg());
        }
    };
}

enum ogg_pcm_type {
    TYPE_VORBIS
};

class ogg_pcm_provider_impl {
    public:
    ogg_pcm_provider_impl() = default;

    ogg_pcm_type type;
    std::chrono::nanoseconds duration;
    bool eof;

    union {
        struct vorbis_type {
            OggVorbis_File vf;
            vorbis_info* info;
        } vorb;
    };
};

ogg_pcm_provider::ogg_pcm_provider(istream_ptr stream) : pcm_provider(std::move(stream))
    , _d { std::make_unique<ogg_pcm_provider_impl>() } {

    auto start = this->stream->tellg();

    bool is_valid = false;
    if (ov_test_callbacks(this->stream.get(), &_d->vorb.vf, nullptr, 0, detail::callbacks) == 0) {
        ov_test_open(&_d->vorb.vf);

        _d->vorb.info = ov_info(&_d->vorb.vf, -1);

        _d->type = TYPE_VORBIS;

        _d->duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            ov_time_total(&_d->vorb.vf, -1) * std::chrono::seconds(1));

        is_valid = true;
    } else {
        ov_clear(&_d->vorb.vf);
        this->stream->seekg(start);
    }

    if (!is_valid) {
        throw std::runtime_error("unkown ogg file type");
    }
}

ogg_pcm_provider::~ogg_pcm_provider() {
    switch (_d->type) {
        case TYPE_VORBIS: ov_clear(&_d->vorb.vf); break;
    }
}

int64_t ogg_pcm_provider::get_samples(void*& data, sample_type type) {
    switch (_d->type) {
        case TYPE_VORBIS: {
            static constexpr size_t data_size = 4096;
            static constexpr size_t word_size = 2;

            switch (type) {
                case SAMPLE_INT16: {
                    data = new char[data_size];
                    int bs;
                    // Get bytes read
                    long size = ov_read(&_d->vorb.vf, static_cast<char*>(data), data_size, 0, word_size, 1, &bs);
                    if (size == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (size < 0) {
                        return PCM_ERR;
                    }

                    return (size / word_size) / _d->vorb.info->channels;
                }

                case SAMPLE_FLOAT32: {
                    int bs;
                    // Array of pointers, 1 per channel
                    float** pcm_target;

                    // Amount of samples per channel
                    long samples = ov_read_float(&_d->vorb.vf, &pcm_target, data_size / word_size, &bs);

                    if (samples == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (samples < 0) {
                        return PCM_ERR;
                    }

                    // ReSharper disable once CppNonReclaimedResourceAcquisition
                    float* dest = new float[samples * static_cast<size_t>(_d->vorb.info->channels)];
                    data = dest;

                    // Interleave
                    for (long i = 0; i < samples; ++i) {
                        for (int j = 0; j < _d->vorb.info->channels; ++j) {
                            *dest++ = pcm_target[j][i];
                        }
                    }

                    return samples;
                }

                case SAMPLE_NONE: return PCM_ERR;
            }
        }
    }

    return PCM_ERR;
}

int64_t ogg_pcm_provider::rate() const {
    switch (_d->type) {
        case TYPE_VORBIS: return _d->vorb.info->rate;
    }

    return 0;
}

int64_t ogg_pcm_provider::channels() const {
    switch (_d->type) {
        case TYPE_VORBIS: return _d->vorb.info->channels;
    }

    return 0;
}

std::chrono::nanoseconds ogg_pcm_provider::duration() const {
    return _d->duration;
}

std::chrono::nanoseconds ogg_pcm_provider::pos() const {
    switch (_d->type) {
        case TYPE_VORBIS: return std::chrono::duration_cast<std::chrono::nanoseconds>(
            ov_time_tell(&_d->vorb.vf) * std::chrono::seconds(1));
    }

    return std::chrono::nanoseconds(0);
}

void ogg_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _validate_open();

    switch (_d->type) {
        case TYPE_VORBIS: ov_time_seek(&_d->vorb.vf,
            static_cast<double>(pos.count()) / std::chrono::nanoseconds::period::den); break;
    }
}


sample_type ogg_pcm_provider::types() const {
    switch (_d->type) {
        case TYPE_VORBIS: return SAMPLE_INT16 | SAMPLE_FLOAT32;
    }

    return SAMPLE_NONE;
}

sample_type ogg_pcm_provider::preferred_type() const {
    switch (_d->type) {
        case TYPE_VORBIS: return SAMPLE_FLOAT32;
    }

    return SAMPLE_NONE;
}

void ogg_pcm_provider::_validate_open() const {
    if (_d->eof) {
        _d->eof = false;

        switch (_d->type) {
            case TYPE_VORBIS: {
                ov_clear(&_d->vorb.vf);

                if (!stream->good()) {
                    stream->clear();
                }
                stream->seekg(0);

                ASSERT(ov_open_callbacks(stream.get(), &_d->vorb.vf, nullptr, 0, detail::callbacks) == 0);
                _d->vorb.info = ov_info(&_d->vorb.vf, -1);
            }
        }
    }
}


sample_type operator|(sample_type left, sample_type right) {
    return static_cast<sample_type>(static_cast<uintmax_t>(left) | static_cast<uintmax_t>(right));
}

