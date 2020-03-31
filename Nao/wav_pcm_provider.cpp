#include "wav_pcm_provider.h"

#include "utils.h"

#include "namespaces.h"

wav_pcm_provider::wav_pcm_provider(const istream_ptr& stream) : pcm_provider(stream)
, _ctx { stream } {

    for (const ffmpeg::avformat::stream& s : _ctx.streams()) {
        if (s.type() == AVMEDIA_TYPE_AUDIO) {
            _stream = s;
            _codec = ffmpeg::avcodec::codec { s.id() };
            break;
        }
    }

    _codec_ctx = ffmpeg::avcodec::context { _codec };
    ASSERT(_codec_ctx.parameters_to_context(_stream));
    ASSERT(_codec_ctx.open(_codec));

    ASSERT(_codec_ctx.sample_format() == AV_SAMPLE_FMT_S16);
}

pcm_samples wav_pcm_provider::get_samples(sample_format type) {
    if (type != SAMPLE_INT16) {
        return pcm_samples::error(PCM_ERR);
    }

    if (!_ctx.read_frame(_packet, _stream.index())) {
        return pcm_samples::error(PCM_ERR);
    }

    if (!_codec_ctx.decode(_packet, _frame)) {
        return pcm_samples::error(PCM_ERR);
    }

    pcm_samples samples { SAMPLE_INT16, _frame.samples(), _frame.channels(), CHANNELS_WAV };

    _samples_played += _frame.samples();

    std::copy_n(_frame.data(), _frame.channels() * _frame.samples() * 2, samples.data());

    return samples;
}

int64_t wav_pcm_provider::rate() {
    return _stream.sample_rate();
}

int64_t wav_pcm_provider::channels() {
    return _stream.channels();
}

channel_order wav_pcm_provider::order() {
    return CHANNELS_WAV;
}

std::string wav_pcm_provider::name() {
    return _codec.long_name();
}

std::chrono::nanoseconds wav_pcm_provider::duration() {
    return _stream.duration();
}

std::chrono::nanoseconds wav_pcm_provider::pos() {
    // Offset to start of data segment
    return _samples_played * std::chrono::nanoseconds { static_cast<int64_t>((1. / _stream.sample_rate()) * 1e9) };
}

void wav_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _samples_played = pos / std::chrono::nanoseconds { static_cast<int64_t>((1. / _stream.sample_rate()) * 1e9) };

    ASSERT(_ctx.seek(pos, _stream.index()));
}

sample_format wav_pcm_provider::types() {
    return SAMPLE_INT16;
}

sample_format wav_pcm_provider::preferred_type() {
    return SAMPLE_INT16;
}

