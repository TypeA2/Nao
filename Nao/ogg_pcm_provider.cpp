#include "ogg_pcm_provider.h"

#include "utils.h"
#include "namespaces.h"

#include "vorbis_decoder.h"
#include "opus_decoder.h"

ogg_pcm_provider::ogg_pcm_provider(const istream_ptr& stream) : pcm_provider(stream) {
    bool is_valid = false;
    
    // Check vorbis
    _m_dec = std::make_unique<vorbis_decoder>(this->stream);
    if (_m_dec->valid()) {
        is_valid = true;
        _m_type = TYPE_VORBIS;
    } else {
        _m_dec.reset();
        this->stream->seekg(0);
    }

    if (!is_valid) {
        // Check opus
        _m_dec = std::make_unique<opus_decoder>(this->stream);
        if (_m_dec->valid()) {
            is_valid = true;
            _m_type = TYPE_OPUS;
        } else {
            _m_dec.reset();
            this->stream->seekg(0);
        }
    }

    if (!is_valid) {
        throw std::runtime_error("unkown ogg file type");
    }
}

pcm_samples ogg_pcm_provider::get_samples(sample_type type) {
    static constexpr int block_size = 1024;

    switch (type) {
        case SAMPLE_INT16: {
            pcm_samples pcm(SAMPLE_INT16, block_size, _m_dec->channels(), CHANNELS_VORBIS);
            int64_t frames = _m_dec->read(pcm.data<SAMPLE_INT16>(), pcm.frames());
            if (frames == 0) {
                return PCM_DONE;
            }

            if (frames < 0) {
                return PCM_ERR;
            }

            pcm.resize(frames);
            return pcm;
        }

        case SAMPLE_FLOAT32: {
            pcm_samples pcm(SAMPLE_FLOAT32, block_size, _m_dec->channels(), CHANNELS_VORBIS);
            int64_t frames = _m_dec->read_float(pcm.data<SAMPLE_FLOAT32>(), pcm.frames());
            if (frames == 0) {
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

    return PCM_ERR;
}

int64_t ogg_pcm_provider::rate() {
    return _m_dec->rate();
}

int64_t ogg_pcm_provider::channels() {
    return _m_dec->channels();
}

channel_order ogg_pcm_provider::order() {
    return CHANNELS_VORBIS;
}

std::string ogg_pcm_provider::name() {
    switch (_m_type) {
        case TYPE_VORBIS: return "Vorbis";
        case TYPE_OPUS: return "Opus";
    }

    return "<error>";
}

std::chrono::nanoseconds ogg_pcm_provider::duration() {
    return _m_dec->duration();
}

std::chrono::nanoseconds ogg_pcm_provider::pos() {
    return _m_dec->pos();
}

void ogg_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _m_dec->seek(pos);
}

sample_type ogg_pcm_provider::types() {
    return SAMPLE_INT16 | SAMPLE_FLOAT32;
}

sample_type ogg_pcm_provider::preferred_type() {
    return SAMPLE_FLOAT32;
}

