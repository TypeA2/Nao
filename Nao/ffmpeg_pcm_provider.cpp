#include "ffmpeg_pcm_provider.h"

ffmpeg_pcm_provider::ffmpeg_pcm_provider(const istream_ptr& stream, const std::string& path)
    : pcm_provider(stream), _ctx { stream, path } {

    // Find first audio stream and use that
    for (const ffmpeg::avformat::stream& s : _ctx.streams()) {
        if (s.type() == AVMEDIA_TYPE_AUDIO) {
            _stream = s;
            break;
        }
    }

    ASSERT(_stream);
    _codec = ffmpeg::avcodec::codec { _stream.id() };
    _codec_ctx = ffmpeg::avcodec::context { _codec };
    ASSERT(_codec_ctx.parameters_to_context(_stream));
    ASSERT(_codec_ctx.open(_codec));

    _fmt = samples::from_avutil(_codec_ctx.sample_format());
    _channel_layout = _codec_ctx.channel_layout();
    _channels = _codec_ctx.channels();
}

pcm_samples ffmpeg_pcm_provider::get_samples() {

    // Read at least 1 frame
    int res;
    do {
        if (!_ctx.read_frame(_packet, _stream.index())) {
            throw pcm_decode_exception("failed to read frame");
        }

        res = _codec_ctx.decode(_packet, _frame);

        if (res != 0 && res != AVERROR(EAGAIN)) {
            throw pcm_decode_exception("failed to decode frame");
        }

        _packet.unref();
    } while (res == AVERROR(EAGAIN));

    pcm_samples samples { _fmt, _frame.samples(), _channels, _channel_layout };

    _samples_played += _frame.samples();

    if (samples::is_planar(_fmt)) {
        // Interleave planar data
        char* const start = samples.data();
        uint8_t channel_count = _frame.channels();
        for (uint8_t i = 0; i < channel_count; ++i) {
            char* cur = start;
            const char* src = _frame[i];

            for (uint64_t j = 0; j < _frame.samples(); ++j) {
                *cur = *src++;

                cur += channel_count;
            }
        }

    } else {
        // Or just straight copy
        samples.fill_n(_frame.data(), samples.bytes());
    }

    return samples;
}

int64_t ffmpeg_pcm_provider::rate() {
    return _stream.sample_rate();
}

int64_t ffmpeg_pcm_provider::channels() {
    return _stream.channels();
}

std::string ffmpeg_pcm_provider::name() {
    return _codec.long_name();
}

std::chrono::nanoseconds ffmpeg_pcm_provider::duration() {
    return _stream.duration();
}

std::chrono::nanoseconds ffmpeg_pcm_provider::pos() {
    // Offset to start of data segment
    return _samples_played * std::chrono::nanoseconds { static_cast<int64_t>((1. / _stream.sample_rate()) * 1e9) };
}

void ffmpeg_pcm_provider::seek(std::chrono::nanoseconds pos) {
    _samples_played = pos / std::chrono::nanoseconds { static_cast<int64_t>((1. / _stream.sample_rate()) * 1e9) };

    ASSERT(_ctx.seek(pos, _stream.index()));
}

sample_format ffmpeg_pcm_provider::format() {
    return _fmt;
}
