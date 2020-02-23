#pragma once

#include "pcm_provider.h"
#include "thread_pool.h"

enum event_type {
    EVENT_START,
    EVENT_STOP
};

class audio_player_pa_lock;

class audio_player {
    public:
    using event_handler = std::function<void()>;

    explicit audio_player(pcm_provider_ptr provider);
    ~audio_player();

    std::chrono::nanoseconds duration() const;
    std::chrono::nanoseconds pos() const;
    void seek(std::chrono::nanoseconds pos) const;

    bool paused() const;
    bool eof() const;
    void pause();
    void play();
    void reset();

    void set_volume_scaled(float val);
    void set_volume_log(float orig, float curve = 2.f);
    float volume_scaled() const;
    float volume_log(float curve = 2.f) const;

    void add_event(event_type type, const event_handler& handler);
    void trigger_event(event_type type) const;

    pcm_provider* provider() const;
    sample_type pcm_format() const;

    private:
    void _playback_loop_passthrough(sample_type output_format);
    void _playback_loop_resample(const PaDeviceInfo* info);
    void _wait_pause();
    void _playback_end();
    void _write_samples(pcm_samples samples) const;

    std::unique_ptr<audio_player_pa_lock> _d;

    pcm_provider_ptr _m_provider;
    thread_pool _m_player;

    bool _m_convert_rate;
    bool _m_convert_format;

    bool _m_quit;
    bool _m_pause;
    bool _m_eof;
    std::mutex _m_pause_mutex;
    std::condition_variable _m_pause_condition;

    std::mutex _m_startup_mutex;
    std::condition_variable _m_startup_condition;
    std::atomic<bool> _m_startup_done;

    float _m_volume;

    PaStream* _m_stream;
    int _m_channel_out;

    std::unordered_map<event_type, std::vector<event_handler>> _m_events;
};
