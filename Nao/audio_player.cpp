#include "audio_player.h"

#include "thread_pool.h"

#include "namespaces.h"

#include <SDL.h>

#include <compare>


audio_player::audio_player(pcm_provider_ptr provider)
    : _provider { std::move(provider) }
    , _device { 44100, AUDIO_S16LSB, 2, 4096,
        std::bind(&audio_player::_audio_callback, this) } {
    _swr = swr_alloc_set_opts(nullptr,
        AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_S16,
        44100,
        AV_CH_LAYOUT_STEREO,
        av_get_packed_sample_fmt(samples::to_avutil(_provider->format())),
        utils::narrow<int>(_provider->rate()), 0, nullptr);

    ASSERT(_swr);
    swr_init(_swr);
}

audio_player::~audio_player() {
    swr_free(&_swr);
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
}

void audio_player::play() {
    _paused = false;
    _device.play();
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

    int out_samples = swr_get_out_samples(_swr, utils::narrow<int>(samples.frames()));
    ASSERT(out_samples > 0);

    std::vector<char> result(out_samples * size_t { 4 }, 0);

    const uint8_t* in = reinterpret_cast<uint8_t*>(samples.data());
    uint8_t* out = reinterpret_cast<uint8_t*>(result.data());

    int converted = swr_convert(_swr, &out, utils::narrow<int>(result.size() / 2),
        &in, utils::narrow<int>(samples.frames()));

    result.resize(converted * size_t { 4 });

    auto data = reinterpret_cast<sample_int16_t*>(result.data());

    for (size_t i = 0; i < (result.size() / 2); ++i) {
        data[i] = utils::narrow<sample_int16_t>(data[i] * _volume);
    }


    return result;
}
