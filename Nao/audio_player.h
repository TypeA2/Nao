#pragma once

#include "pcm_provider.h"
#include "thread_pool.h"

#include "sdl2.h"

enum event_type {
    EVENT_START,
    EVENT_STOP
};

class audio_player {
    public:
    using event_handler = std::function<void()>;

    private:
    pcm_provider_ptr _provider;
    sdl::audio::device _device;

    pcm_samples _current_samples = pcm_samples::error(-1);
    int64_t _already_consumed = 0;

    float _volume = 1.f;
    bool _eof = false;
    bool _paused = true;

    std::unordered_map<event_type, std::vector<event_handler>> _events;

    public:
    explicit audio_player(pcm_provider_ptr provider);
    ~audio_player();

    std::chrono::nanoseconds duration() const;
    std::chrono::nanoseconds pos() const;
    void seek(std::chrono::nanoseconds pos) const;

    bool paused() const;
    bool eof() const;
    void pause();
    void play();
    void reset() {};

    void set_volume_scaled(float val);
    void set_volume_log(float orig, float curve = 2.f);
    float volume_scaled() const;
    float volume_log(float curve = 2.f) const;

    void add_event(event_type type, const event_handler& handler);
    void trigger_event(event_type type) const;

    pcm_provider* provider() const;
    sample_format pcm_format() const;

    private:
    void _audio_callback(uint8_t* buf, int len);
};
