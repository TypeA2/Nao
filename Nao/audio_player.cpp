#include "audio_player.h"

#include "thread_pool.h"

#include "namespaces.h"

#include <SDL.h>

namespace detail {
    static constexpr int out_sample_rate = 48000;
    static constexpr SDL_AudioFormat sdl_out_sample_format = AUDIO_S16LSB;
    static constexpr AVSampleFormat av_out_sample_format = AV_SAMPLE_FMT_S16;
    static constexpr int64_t out_channel_count = 2;
    static constexpr uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    static constexpr int out_buffer_size = 4096;

    static constexpr size_t out_sample_size = 2;

    static constexpr ffmpeg::swresample::context::audio_info output_audio {
        .channel_layout = out_channel_layout,
        .sample_format = av_out_sample_format,
        .sample_rate = out_sample_rate
    };
}

audio_player::audio_player(pcm_provider_ptr provider)
    : _provider { std::move(provider) }
    , _device { detail::out_sample_rate, detail::sdl_out_sample_format, detail::out_channel_count,
        detail::out_buffer_size, std::bind(&audio_player::_audio_callback, this) }
    , _swr { { av_get_default_channel_layout(utils::narrow<int>(_provider->channels())),
                av_get_packed_sample_fmt(samples::to_av(_provider->format())),
                utils::narrow<int>(_provider->rate())  },
            detail::output_audio } {

}

audio_player::~audio_player() {
    trigger_event(EVENT_STOP);
}

std::chrono::nanoseconds audio_player::duration() const {
    return _provider->duration();
}

std::chrono::nanoseconds audio_player::pos() const {
    return _provider->pos();
}

void audio_player::seek(std::chrono::nanoseconds pos) const {
    _provider->seek(pos);
}

bool audio_player::paused() const {
    return _paused;
}

bool audio_player::eof() const {
    return _eof;
}

void audio_player::pause() {
    _paused = true;
    _device.pause();

    trigger_event(EVENT_STOP);
}

void audio_player::play() {
    _paused = false;
    _device.play();

    trigger_event(EVENT_START);
}


void audio_player::set_volume_scaled(float val) {
    _volume = std::clamp(val, 0.f, 1.f);
}

void audio_player::set_volume_log(float orig, float curve) {
    _volume = std::clamp(pow(orig, curve), 0.f, 1.f);
}

float audio_player::volume_scaled() const {
    return _volume;
}

float audio_player::volume_log(float curve) const {
    return std::clamp(pow(_volume, 1.f / curve), 0.f, 1.f);
}

void audio_player::add_event(event_type type, const event_handler& handler) {
    if (handler) {
        _events[type].push_back(handler);
    }
}

void audio_player::trigger_event(event_type type) const {
    if (_events.find(type) == _events.end()) {
        return;
    }

    for (const event_handler& func : _events.at(type)) {
        func();
    }
}

pcm_provider* audio_player::provider() const {
    return _provider.get();
}

sample_format audio_player::pcm_format() const {
    return _provider->format();
}

std::vector<char> audio_player::_audio_callback() {
    (void) this;
    pcm_samples samples = _provider->get_samples();

    if (!samples) {
        _eof = true;
        pause();
        return { };
    }

    int64_t max_out_frames = _swr.frames_for_input(samples.frames());
    std::vector<char> result(max_out_frames * detail::out_sample_size * detail::out_channel_count);

    char* in = samples.data();
    char* out = result.data();
    int64_t converted = _swr.convert(&in, samples.frames(), &out, max_out_frames);

    result.resize(converted * detail::out_sample_size * detail::out_channel_count);

    auto data = reinterpret_cast<sample_int16_t*>(result.data());

    for (int64_t i = 0; i < (converted * detail::out_channel_count); ++i) {
        data[i] = utils::narrow<sample_int16_t>(data[i] * _volume);
    }

    return result;
}
