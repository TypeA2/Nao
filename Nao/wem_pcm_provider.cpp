#include "wem_pcm_provider.h"

#include "riff.h"
#include "wwriff.h"

#include "utils.h"

#include <ogg/ogg.h>

#include "frameworks.h"
#include "resource.h"

#include "namespaces.h"

#include "vorbis_decoder.h"

wem_pcm_provider::wem_pcm_provider(const istream_ptr& stream) : pcm_provider(stream) {
    _m_buf = std::make_shared<binary_iostream>(
        std::make_unique<std::stringstream>(std::ios::in | std::ios::out | std::ios::binary));

    stream->seekg(0);
    ASSERT(wwriff::wwriff_to_ogg(stream, _m_buf));
;
    _m_buf->seekg(0);

    _m_dec = std::make_unique<vorbis_decoder>(_m_buf);
    ASSERT(_m_dec->valid());
}

pcm_samples wem_pcm_provider::get_samples(sample_type type) {
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

int64_t wem_pcm_provider::rate() {
    return _m_dec->rate();
}

int64_t wem_pcm_provider::channels() {
    return _m_dec->channels();
}

channel_order wem_pcm_provider::order() {
    return CHANNELS_VORBIS;
}

std::string wem_pcm_provider::name() {
    return "Vorbis (Audiokinetic Wwise)";
}

std::chrono::nanoseconds wem_pcm_provider::duration() {
    return _m_dec->duration();
}

std::chrono::nanoseconds wem_pcm_provider::pos() {
    return _m_dec->pos();
}

void wem_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _m_dec->seek(pos);
}

sample_type wem_pcm_provider::types() {
    return SAMPLE_INT16 | SAMPLE_FLOAT32;
}

sample_type wem_pcm_provider::preferred_type() {
    return SAMPLE_FLOAT32;
}

