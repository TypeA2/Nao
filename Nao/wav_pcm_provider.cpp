#include "wav_pcm_provider.h"

#include "utils.h"

#include "namespaces.h"

wav_pcm_provider::wav_pcm_provider(const istream_ptr& stream) : pcm_provider(stream)
    , _m_fmt { }, _m_data { }, _m_ns_per_frame { } {
    
    // WAVE header
    stream->read(&_m_wave, sizeof(_m_wave));

    ASSERT(std::string(_m_wave.riff.header, 4) == "RIFF");
    ASSERT(std::string( _m_wave.wave, 4) == "WAVE");

    // Search for fmt and data chunks
    bool fmt_read = false;
    bool data_read = false;
    while (!fmt_read || !data_read) {
        riff_header riff;
        stream->read(&riff, sizeof(riff));

        if (std::string(riff.header, 4) == "fmt "s) {
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

        if (std::string(riff.header, 4) == "data") {
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

pcm_samples wav_pcm_provider::get_samples(sample_type type) {
    if (type != SAMPLE_INT16) {
        return pcm_samples::error(PCM_ERR);
    }

    pcm_samples samples { SAMPLE_INT16, frames_per_block, _m_fmt.channels, CHANNELS_WAV };
    stream->read(samples.data(), 2, frames_per_block * _m_fmt.channels);

    // Maybe didn't read entire block
    samples.resize(stream->gcount() / samples.frame_size());

    return samples;
}

int64_t wav_pcm_provider::rate() {
    return _m_fmt.rate;
}

int64_t wav_pcm_provider::channels() {
    return _m_fmt.channels;
}

channel_order wav_pcm_provider::order() {
    return CHANNELS_WAV;
}

std::chrono::nanoseconds wav_pcm_provider::duration() {
    // align is frame size, return the number of frames multiplied by nanoseconds per frame
    return (_m_data.size / _m_fmt.align) * _m_ns_per_frame;
}

std::chrono::nanoseconds wav_pcm_provider::pos() {
    // Offset to start of data segment
    return ((stream->tellg() - _m_data_start) / _m_fmt.align) * _m_ns_per_frame;
}

void wav_pcm_provider::seek(std::chrono::nanoseconds pos) {
    // Seek to whole frame
    stream->seekg(_m_data_start + ((pos / _m_ns_per_frame) * _m_fmt.align));
}

sample_type wav_pcm_provider::types() {
    return SAMPLE_INT16;
}

sample_type wav_pcm_provider::preferred_type() {
    return SAMPLE_INT16;
}

