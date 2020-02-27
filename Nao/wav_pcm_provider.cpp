#include "wav_pcm_provider.h"

#include "utils.h"

#include "namespaces.h"

wav_pcm_provider::wav_pcm_provider(const istream_ptr& stream) : pcm_provider(stream) {
    riff_header hdr;
    stream->read(&hdr, sizeof(hdr));
    ASSERT(std::string(hdr.header, 4) == "RIFF");

    wave_chunk wave;
    stream->read(&wave, sizeof(wave));
    ASSERT(std::string(wave.wave, 4) == "WAVE");

    // Search for fmt and data chunks
    bool fmt_read = false;
    bool data_read = false;
    while (!fmt_read || !data_read) {
        riff_header riff;
        stream->read(&riff, sizeof(riff));

        if (std::string(riff.header, 4) == "fmt ") {
            // Format chunk
            stream->read(&_fmt, sizeof(_fmt));

            ASSERT(_fmt.format == 1 || _fmt.format == 0xFFFE);
            ASSERT(_fmt.bits == 16); // SAMPLE_INT16 only

            if (_fmt.format == 0xFFFE) {
                stream->read(&_fmt_ex, sizeof(_fmt_ex));
                ASSERT(_fmt_ex.valid_bits == 16);
                stream->seekg(_fmt_ex.extra_size - sizeof(_fmt_ex) + sizeof(_fmt_ex.extra_size), std::ios::cur);
            }

            _ns_per_frame = std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(
                (1. / _fmt.rate) * std::chrono::nanoseconds::period::den));

            fmt_read = true;
            continue;
        }

        if (std::string(riff.header, 4) == "data") {
            // data chunk
            _data_start = stream->tellg();
            _data = riff;
            data_read = true;
            break;
        }

        // Skip chunk
        stream->seekg(riff.size, std::ios::cur);
    }

    ASSERT(fmt_read && data_read);

    // Everything read

    stream->seekg(_data_start);
}

pcm_samples wav_pcm_provider::get_samples(sample_type type) {
    if (type != SAMPLE_INT16) {
        return pcm_samples::error(PCM_ERR);
    }

    pcm_samples samples { SAMPLE_INT16, frames_per_block, _fmt.channels, CHANNELS_WAV };
    stream->read(samples.data(), 2, frames_per_block * _fmt.channels);

    // Maybe didn't read entire block
    samples.resize(stream->gcount() / samples.frame_size());

    return samples;
}

int64_t wav_pcm_provider::rate() {
    return _fmt.rate;
}

int64_t wav_pcm_provider::channels() {
    return _fmt.channels;
}

channel_order wav_pcm_provider::order() {
    return CHANNELS_WAV;
}

std::string wav_pcm_provider::name() {
    switch (_fmt.format) {
        case 1:      return "WAVE";
        case 0xFFFE: return "WAVE Extensible";
        default: break;
    }

    return "<error>";
}

std::chrono::nanoseconds wav_pcm_provider::duration() {
    // align is frame size, return the number of frames multiplied by nanoseconds per frame
    return (_data.size / _fmt.align) * _ns_per_frame;
}

std::chrono::nanoseconds wav_pcm_provider::pos() {
    // Offset to start of data segment
    return ((stream->tellg() - _data_start) / _fmt.align) * _ns_per_frame;
}

void wav_pcm_provider::seek(std::chrono::nanoseconds pos) {
    // Seek to whole frame
    stream->seekg(_data_start + ((pos / _ns_per_frame) * _fmt.align));
}

sample_type wav_pcm_provider::types() {
    return SAMPLE_INT16;
}

sample_type wav_pcm_provider::preferred_type() {
    return SAMPLE_INT16;
}

