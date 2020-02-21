#include "wav_pcm_provider.h"

#include "utils.h"

wav_pcm_provider::wav_pcm_provider(const istream_ptr& stream) : pcm_provider(stream)
    , _m_fmt { }, _m_data { }, _m_ns_per_frame { } {
    
    // WAVE header
    stream->read(&_m_wave, sizeof(_m_wave));
    ASSERT(memcmp(_m_wave.riff.header, "RIFF", 4) == 0);
    ASSERT(memcmp(_m_wave.wave, "WAVE", 4) == 0);

    // Search for fmt and data chunks
    bool fmt_read = false;
    bool data_read = false;
    while (!fmt_read || !data_read) {
        riff_header riff;
        stream->read(&riff, sizeof(riff));

        if (memcmp(riff.header, "fmt ", 4) == 0) {
            // Format chunk
            stream->rseek(-1 * sizeof(riff));
            stream->read(&_m_fmt, sizeof(_m_fmt));

            ASSERT(_m_fmt.riff.size == 16); // PCM only
            ASSERT(_m_fmt.format == 1);

            ASSERT(_m_fmt.bits == 16); // SAMPLE_INT16 only

            _m_ns_per_frame = std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(
                (1. / _m_fmt.rate) * std::chrono::nanoseconds::period::den));

            fmt_read = true;
            continue;
        }

        if (memcmp(riff.header, "data", 4) == 0) {
            // data chunk
            _m_data_start = stream->tellg();
            _m_data = riff;
            data_read = true;
            continue;
        }

        // Skip chunk
        stream->seekg(riff.size, std::ios::cur);
    }

    // Everything read

    stream->seekg(_m_data_start);
}

wav_pcm_provider::~wav_pcm_provider() {
    
}

int64_t wav_pcm_provider::get_samples(void*& data, sample_type type) {
    if (type != SAMPLE_INT16) {
        return PCM_ERR;
    }

    data = new int16_t[frames_per_block * _m_fmt.channels];
    stream->read(data, 2, frames_per_block * _m_fmt.channels);

    return stream->gcount() / 2 / _m_fmt.channels;
}

int64_t wav_pcm_provider::rate() const {
    return _m_fmt.rate;
}

int64_t wav_pcm_provider::channels() const {
    return _m_fmt.channels;
}

std::chrono::nanoseconds wav_pcm_provider::duration() const {
    // align is frame size, return the number of frames multiplied by nanoseconds per frame
    return (_m_data.size / _m_fmt.align) * _m_ns_per_frame;
}

std::chrono::nanoseconds wav_pcm_provider::pos() const {
    // Offset to start of data segment
    return ((stream->tellg() - _m_data_start) / _m_fmt.align) * _m_ns_per_frame;
}

void wav_pcm_provider::seek(std::chrono::nanoseconds pos) {
    // Seek to whole frame
    stream->seekg(_m_data_start + ((pos / _m_ns_per_frame) * _m_fmt.align));
}

sample_type wav_pcm_provider::types() const {
    return SAMPLE_INT16;
}

sample_type wav_pcm_provider::preferred_type() const {
    return SAMPLE_INT16;
}

