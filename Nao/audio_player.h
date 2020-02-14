#pragma once

#include "binary_stream.h"
#include "frameworks.h"

#include <functional>
#include <map>

enum playback_state {
    STATE_STOPPED,
    STATE_PENDING,

    STATE_PAUSED,
    STATE_PLAYING,

    STATE_CLOSING
};

enum event_type {
    EVENT_PAUSE,
    EVENT_RESTART,
    EVENT_SET_RATE,
    EVENT_START,
    EVENT_STOP
};

class nao_controller;

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
