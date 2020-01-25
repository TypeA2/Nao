#pragma once

#include "ui_element.h"

#include <string>
#include <memory>

#include <mfidl.h>

class data_model;
class push_button;
class binary_stream;

struct IMFMediaSession;
struct IMFMediaSource;
struct IMFPresentationDescriptor;
struct IMFTopology;

struct item_data;

class audio_player : public ui_element, IMFAsyncCallback {
#pragma region IMF interfaces
    public:
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override;

    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) override;
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;

    private:
    volatile uint32_t _m_refcount;

#pragma endregion
    public:
    audio_player(ui_element* parent, data_model& model);
    ~audio_player();

    void set_audio(const std::string& name,
        const std::shared_ptr<binary_stream>& stream);

    HRESULT pause();
    HRESULT play();

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;

    private:
    enum player_state {
        PlayerStateClosed,
        PlayerStateReady,
        PlayerStatePending,
        PlayerStateStarted,
        PlayerStatePaused,
        PlayerStateStopped,
        PlayerStateClosing
    };

    static constexpr UINT WM_APP_MSG = WM_APP + 1;

    void _init();

    void _toggle_pause_play();
    HRESULT _start_playback();

    void _create_session();
    void _close_session();
    void _create_source(const std::string& name,
        const std::shared_ptr<binary_stream>& stream);

    IMFTopology* _create_topology(IMFPresentationDescriptor* pd);

    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT _player_event(WPARAM wparam, LPARAM lparam);

    data_model& _m_model;

    push_button* _m_toggle;

    HICON _m_play_icon;
    HICON _m_pause_icon;

    IMFMediaSession* _m_session;
    IMFMediaSource* _m_source;
    HANDLE _m_close_event;

    player_state _m_state;

    bool _m_is_playing;
};