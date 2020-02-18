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

    private:
    void _playback_loop_passthrough(sample_type output_format);
    void _playback_loop_resample(const PaDeviceInfo* info);
    void _write_samples(void* samples, int64_t frames, int64_t channels, sample_type type) const;

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

    std::unordered_map<event_type, std::vector<event_handler>> _m_events;
};

/*
class audio_player : IMFAsyncCallback, IMFClockStateSink {
    public:
    explicit audio_player(const istream_ptr& stream, const std::string& path, nao_controller& controller);
    virtual ~audio_player();

    void toggle_playback();
    playback_state state() const;

    void set_volume_scaled(float val) const;
    void set_volume_log(float orig, float curve = 2.f) const;
    float get_volume_scaled() const;
    float get_volume_log(float curve = 2.f) const;

    std::chrono::nanoseconds get_duration() const;
    std::chrono::nanoseconds get_current_time() const;

    const std::string& get_mime_type() const;
    uint32_t get_bitrate() const;

    void seek(std::chrono::nanoseconds to, bool resume = true);

    void add_event(event_type type, const std::function<void()>& func);
    void trigger_event(event_type type) const;

    private:
    void _handle_event(const com_ptr<IMFMediaEvent>& event, MediaEventType type);
    com_ptr<IMFTopology> _create_topology(const com_ptr<IMFPresentationDescriptor>& pd) const;

    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;

    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) override;
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;

    STDMETHODIMP OnClockPause(MFTIME) override;
    STDMETHODIMP OnClockRestart(MFTIME) override;
    STDMETHODIMP OnClockSetRate(MFTIME, float) override;
    STDMETHODIMP OnClockStart(MFTIME, LONGLONG) override;
    STDMETHODIMP OnClockStop(MFTIME) override;

    // Functions to call on these events
    std::map<event_type, std::vector<std::function<void()>>> _m_event_handlers;

    protected:
    nao_controller& controller;

    private:
    volatile uint32_t _m_ref_count;

    istream_ptr _m_stream;
    std::string _m_path;
    
    playback_state _m_playback_state;
    com_ptr<IMFMediaSession> _m_session;
    com_ptr<IMFMediaSource> _m_source;
    com_ptr<IMFSimpleAudioVolume> _m_volume;
    com_ptr<IMFPresentationClock> _m_clock;

    std::chrono::nanoseconds _m_duration;

    // Used to wait untill basic setup is finished
    std::mutex _m_mutex;
    std::condition_variable _m_condition;

    std::string _m_mime_type;
    uint32_t _m_bitrate;

    static com_ptr<IAudioEndpointVolume> _default_endpoint_volume;
};
*/