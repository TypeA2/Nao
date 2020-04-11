#include "audio_player.h"

#include "thread_pool.h"

#include "namespaces.h"

#include <SDL.h>

#include <compare>


audio_player::audio_player(pcm_provider_ptr provider)
    : _provider { std::move(provider) }
    , _device { 44100, AUDIO_S16LSB, 2, 1024,
        std::bind(&audio_player::_audio_callback, this, std::placeholders::_1, std::placeholders::_2) } {

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
    return SAMPLE_INT16;
}

void audio_player::_audio_callback(uint8_t* buf, int len) {
    int64_t consumed = 0;

    if (_already_consumed > 0) {
        // Remaining samples from previous invocation
        int64_t remaining = _current_samples.samples() * 2 - _already_consumed;

        if (remaining < len) {
            std::copy_n(_current_samples.data() + _already_consumed, remaining, buf);
            consumed += remaining;
            _already_consumed = 0;
        } else {
            // Need to truncate again
            std::copy_n(_current_samples.data() + _already_consumed, len, buf);
            _already_consumed -= len;
            return;
        }
    }

    do {
        _current_samples = _provider->get_samples(SAMPLE_INT16).downmix(2);
        _current_samples.scale(_volume);

        int64_t this_time = std::min<int64_t>(_current_samples.samples() * 2, len - consumed);
        std::copy_n(_current_samples.data(), this_time, buf + consumed);

        consumed += this_time;

        if (this_time != _current_samples.samples() * 2) {
            // Buffer full
            _already_consumed = this_time;
        }
    } while (consumed < len);
}
