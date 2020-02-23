#include "ogg_pcm_provider.h"

#include <vorbis/vorbisfile.h>
#include <opusfile.h>

#include "namespaces.h"

#include "utils.h"

namespace detail {
    static ov_callbacks vorbis_callbacks {
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

    static OpusFileCallbacks opus_callbacks {
        .read = [](void* source, unsigned char* data, int count) -> int {
            binary_istream& stream = *static_cast<binary_istream*>(source);

            if (!stream.good() || stream.eof()) {
                return 0;
            }

            return static_cast<int>(stream.read(data, count).gcount());
        },

        .seek = [](void* source, opus_int64 offset, int whence) {
            binary_istream& stream = *static_cast<binary_istream*>(source);

            switch (whence) {
                case SEEK_SET: stream.seekg(offset, std::ios::beg); break;
                case SEEK_CUR: stream.seekg(offset, std::ios::cur); break;
                case SEEK_END: stream.seekg(offset, std::ios::end); break;
                default: return -1;
            }

            return 0;
        },

        .tell = [](void* source) -> opus_int64 {
            return static_cast<binary_istream*>(source)->tellg();
        },

        .close = [](void*) -> int { return 0; }
    };
}

enum ogg_pcm_type {
    TYPE_VORBIS,
    TYPE_OPUS
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

        struct opus_type {
            OggOpusFile* of;
            OpusHead const* head;
            std::chrono::nanoseconds ns_per_sample;
        } opus;
    };
};

ogg_pcm_provider::ogg_pcm_provider(const istream_ptr& stream) : pcm_provider(stream)
    , _d { std::make_unique<ogg_pcm_provider_impl>() } {

    auto start = this->stream->tellg();

    bool is_valid = false;

    // Check vorbis
    if (ov_test_callbacks(this->stream.get(), &_d->vorb.vf, nullptr, 0, detail::vorbis_callbacks) == 0) {
        ov_test_open(&_d->vorb.vf);

        _d->vorb.info = ov_info(&_d->vorb.vf, -1);

        _d->type = TYPE_VORBIS;

        _d->duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            ov_time_total(&_d->vorb.vf, -1) * 1s);

        is_valid = true;
    } else {
        ov_clear(&_d->vorb.vf);
        this->stream->seekg(start);
    }

    // Check opus
    if (OggOpusFile* of = op_test_callbacks(this->stream.get(), &detail::opus_callbacks,
        nullptr, 0, nullptr); of != nullptr) {

        op_test_open(of);

        _d->opus.of = of;
        _d->opus.head = op_head(of, -1);
        _d->opus.ns_per_sample = std::chrono::duration_cast<std::chrono::nanoseconds>(
            1s / static_cast<double>(_d->opus.head->input_sample_rate));

        _d->type = TYPE_OPUS;

        _d->duration = op_pcm_total(of, -1) * _d->opus.ns_per_sample;

        is_valid = true;
    }

    if (!is_valid) {
        throw std::runtime_error("unkown ogg file type");
    }
}

ogg_pcm_provider::~ogg_pcm_provider() {
    switch (_d->type) {
        case TYPE_VORBIS: ov_clear(&_d->vorb.vf); break;
        case TYPE_OPUS: op_free(_d->opus.of); break;
    }
}

pcm_samples ogg_pcm_provider::get_samples(sample_type type) {
    static constexpr size_t block_size = 1024;
    static constexpr size_t word_size = 2;

    switch (_d->type) {
        case TYPE_VORBIS: {
            switch (type) {
                case SAMPLE_INT16: {
                    pcm_samples pcm(SAMPLE_INT16, block_size, _d->vorb.info->channels, CHANNELS_VORBIS);

                    int bs;
                    // Get bytes read
                    long size = ov_read(&_d->vorb.vf, pcm.data(), static_cast<int>(pcm.frame_size() * pcm.frames()), 0, word_size, 1, &bs);
                    if (size == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (size < 0) {
                        return PCM_ERR;
                    }

                    return pcm;
                }

                case SAMPLE_FLOAT32: {
                    int bs;
                    // Array of pointers, 1 per channel
                    float** pcm_target;

                    // Amount of samples per channel
                    long samples = ov_read_float(&_d->vorb.vf, &pcm_target, static_cast<int>(block_size) * _d->vorb.info->channels, &bs);

                    if (samples == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (samples < 0) {
                        return PCM_ERR;
                    }

                    pcm_samples pcm(SAMPLE_FLOAT32, samples, _d->vorb.info->channels, CHANNELS_VORBIS);
                    sample_float32_t* dest = pcm.data<SAMPLE_FLOAT32>();

                    // Interleave
                    for (long i = 0; i < samples; ++i) {
                        for (int j = 0; j < _d->vorb.info->channels; ++j) {
                            *dest++ = pcm_target[j][i];
                        }
                    }

                    return pcm;
                }

                case SAMPLE_NONE: return PCM_ERR;
            }
            break;
        }

        case TYPE_OPUS: {
            switch (type) {
                case SAMPLE_INT16: {
                    pcm_samples pcm(SAMPLE_INT16, block_size, _d->opus.head->channel_count, CHANNELS_VORBIS);
                    int frames = op_read(_d->opus.of, pcm.data<SAMPLE_INT16>(), static_cast<int>(pcm.samples()), nullptr);
                    if (frames == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (frames < 0) {
                        return PCM_ERR;
                    }

                    pcm.resize(frames);

                    return pcm;
                }

                case SAMPLE_FLOAT32: {
                    pcm_samples pcm(SAMPLE_FLOAT32, block_size, _d->opus.head->channel_count, CHANNELS_VORBIS);

                    int frames = op_read_float(_d->opus.of, pcm.data<SAMPLE_FLOAT32>(), static_cast<int>(pcm.samples()), nullptr);
                    if (frames == 0) {
                        _d->eof = true;
                        return PCM_DONE;
                    }

                    if (frames < 0) {
                        return PCM_ERR;
                    }

                    pcm.resize(frames);

                    return pcm;
                }

                case SAMPLE_NONE: return PCM_ERR;
            }
            break;
        }
    }

    return PCM_ERR;
}

int64_t ogg_pcm_provider::rate() {
    switch (_d->type) {
        case TYPE_VORBIS: return _d->vorb.info->rate;
        case TYPE_OPUS: return _d->opus.head->input_sample_rate;
    }

    return 0;
}

int64_t ogg_pcm_provider::channels() {
    switch (_d->type) {
        case TYPE_VORBIS: return _d->vorb.info->channels;
        case TYPE_OPUS: return _d->opus.head->channel_count;
    }

    return 0;
}

channel_order ogg_pcm_provider::order() {
    return CHANNELS_VORBIS;
}

std::string ogg_pcm_provider::name() {
    switch (_d->type) {
        case TYPE_VORBIS: return "Vorbis";
        case TYPE_OPUS: return "Opus";
    }

    return "<error>";
}

std::chrono::nanoseconds ogg_pcm_provider::duration() {
    return _d->duration;
}

std::chrono::nanoseconds ogg_pcm_provider::pos() {
    switch (_d->type) {
        case TYPE_VORBIS: return std::chrono::duration_cast<std::chrono::nanoseconds>(ov_time_tell(&_d->vorb.vf) * 1s);
        case TYPE_OPUS: return op_pcm_tell(_d->opus.of) * _d->opus.ns_per_sample;
    }

    return 0ns;
}

void ogg_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _validate_open();

    switch (_d->type) {
        case TYPE_VORBIS: ov_time_seek(&_d->vorb.vf,
            static_cast<double>(pos.count()) / std::chrono::nanoseconds::period::den); break;
        case TYPE_OPUS: op_pcm_seek(_d->opus.of, pos / _d->opus.ns_per_sample);
    }
}

sample_type ogg_pcm_provider::types() {
    return SAMPLE_INT16 | SAMPLE_FLOAT32;
}

sample_type ogg_pcm_provider::preferred_type() {
    return SAMPLE_FLOAT32;
}

void ogg_pcm_provider::_validate_open() const {
    if (_d->eof) {
        _d->eof = false;

        switch (_d->type) {
            case TYPE_VORBIS: {
                ov_clear(&_d->vorb.vf);

                stream->seekg(0);

                ASSERT(ov_open_callbacks(stream.get(), &_d->vorb.vf, nullptr, 0, detail::vorbis_callbacks) == 0);
                _d->vorb.info = ov_info(&_d->vorb.vf, -1);
                break;
            }

            case TYPE_OPUS: {
                op_free(_d->opus.of);

                stream->seekg(0);

                _d->opus.of = op_open_callbacks(stream.get(), &detail::opus_callbacks, nullptr, 0, nullptr);
                ASSERT(_d->opus.of);

                _d->opus.head = op_head(_d->opus.of, -1);
                break;
            }
        }
    }
}
