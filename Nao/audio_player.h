#pragma once

#include "binary_stream.h"
#include "frameworks.h"

enum playback_state {
    STATE_STOPPED,
    STATE_PENDING,

    STATE_PAUSED,
    STATE_PLAYING,

    STATE_CLOSING
};

class nao_controller;

class audio_player : IMFAsyncCallback, IAudioEndpointVolumeCallback {
    public:
    explicit audio_player(const istream_ptr& stream, const std::string& path, nao_controller& controller);
    virtual ~audio_player();

    void toggle_playback();
    playback_state state() const;

    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;

    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) override;
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;

    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

    void set_volume_percent(float val) const;

    private:
    void _handle_event(IMFMediaEvent* event, MediaEventType type);
    IMFTopology* _create_topology(IMFPresentationDescriptor* pd);

    protected:
    nao_controller& controller;

    private:
    volatile uint32_t _m_ref_count;

    istream_ptr _m_stream;
    std::string _m_path;
    
    playback_state _m_playback_state;
    IMFMediaSession* _m_session;
    IMFMediaSource* _m_source;
    IMFSimpleAudioVolume* _m_volume;

    float _m_volume_scalar;

    static IAudioEndpointVolume* _default_endpoint_volume;
};
