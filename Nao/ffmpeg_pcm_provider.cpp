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

    switch (_codec_ctx.sample_format()) {
        case AV_SAMPLE_FMT_FLT:
            _is_float = true;
        case AV_SAMPLE_FMT_S16:
            break;

        case AV_SAMPLE_FMT_FLTP:
            _is_float = true;
        case AV_SAMPLE_FMT_S16P:
            _is_planar = true;
            break;

        default: ASSERT(false);
    }
}

pcm_samples ffmpeg_pcm_provider::get_samples(sample_format type) {
    if (type != SAMPLE_INT16 && type != SAMPLE_FLOAT32) {
        return pcm_samples::error(PCM_ERR);
    }

    int res;
    do {
        if (!_ctx.read_frame(_packet, _stream.index())) {
            return pcm_samples::error(PCM_ERR);
        }

        res = _codec_ctx.decode(_packet, _frame);

        if (res != 0 && res != AVERROR(EAGAIN)) {
            return pcm_samples::error(PCM_ERR);
        }
    } while (res == AVERROR(EAGAIN));

    pcm_samples samples { !_is_float ? SAMPLE_INT16 : SAMPLE_FLOAT32, _frame.samples(), _frame.channels(), CHANNELS_WAV };

    _samples_played += _frame.samples();

    if (_is_planar) {
        if (_is_float) {
            sample_float32_t* dest = samples.data<SAMPLE_FLOAT32>();
            for (int64_t i = 0; i < _frame.samples(); ++i) {
                for (int64_t j = 0; j < _stream.channels(); ++j) {
                    *dest++ = reinterpret_cast<const sample_float32_t*>(_frame.data(j))[i];
                }
            }
        } else {
            sample_int16_t* dest = samples.data<SAMPLE_INT16>();
            for (int64_t i = 0; i < _frame.samples(); ++i) {
                for (int64_t j = 0; j < _stream.channels(); ++j) {
                    *dest++ = reinterpret_cast<const sample_int16_t*>(_frame.data(j))[i];
                }
            }
        }
    } else {
        std::copy_n(_frame.data(), _frame.channels() * _frame.samples() * (_is_float ? 4 : 2), samples.data());
    }

    return samples;
}

int64_t ffmpeg_pcm_provider::rate() {
    return _stream.sample_rate();
}

int64_t ffmpeg_pcm_provider::channels() {
    return _stream.channels();
}

channel_order ffmpeg_pcm_provider::order() {
    return CHANNELS_WAV;
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

sample_format ffmpeg_pcm_provider::types() {
    return _is_float ? SAMPLE_FLOAT32 : SAMPLE_INT16;
}

sample_format ffmpeg_pcm_provider::preferred_type() {
    return _is_float ? SAMPLE_FLOAT32 : SAMPLE_INT16;
}