#include "ffmpeg_pcm_provider.h"

#include "namespaces.h"

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

    _fmt = samples::from_av(_codec_ctx.sample_format());
    _channel_layout = _codec_ctx.channel_layout();
    _channels = _codec_ctx.channels();
}

pcm_samples ffmpeg_pcm_provider::get_samples() {

    // Read at least 1 frame
    int res;
    do {
        res = _ctx.read_frame(_packet, _stream.index());
        if (res != 0) {
            // EOF
            return { _fmt, 0, _channels, _channel_layout };
        }

        res = _codec_ctx.decode(_packet, _frame);

        if (res != 0 && res != AVERROR(EAGAIN)) {
            char buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(res, buf, AV_ERROR_MAX_STRING_SIZE);
            throw pcm_decode_exception("failed to decode frame "s + buf);
        }

        _packet.unref();
    } while (res == AVERROR(EAGAIN));

    pcm_samples samples { _fmt, _frame.samples(), _channels, _channel_layout };

    _samples_played += _frame.samples();

    if (samples::is_planar(_fmt)) {
        // Interleave planar data
        size_t sample_size = av_get_bytes_per_sample(samples::to_av(_fmt));
        char* dest = samples.data();
        for (uint64_t i = 0; i < _frame.samples(); ++i) {
            for (uint8_t j = 0; j < _channels; ++j) {
                std::copy_n(_frame[j] + sample_size * i, sample_size, dest);
                dest += sample_size;
            }
        }

    } else {
        // Or just straight copy
        samples.fill_n(_frame.data(), samples.bytes());
    }

    return samples;
}

int64_t ffmpeg_pcm_provider::rate() {
    return _codec_ctx.sample_rate();
}

int64_t ffmpeg_pcm_provider::channels() {
    return _codec_ctx.channels();
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
