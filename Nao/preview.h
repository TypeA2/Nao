#pragma once

#include "file_handler.h"
#include "nao_view.h"

#include "audio_player.h"

#include <chrono>

#include "mf.h"

class nao_controller;

// Wrapper class for preview elements

class preview : public ui_element {
    public:
    explicit preview(nao_view& view);
    virtual ~preview() = default;

    protected:
    nao_view& view;
    nao_controller& controller;
};

using preview_ptr = std::unique_ptr<preview>;

class list_view;

// Multi-use list-view preview
class list_view_preview : public preview {
    item_file_handler* _handler;

    std::unique_ptr<list_view> _list;

    // Current column ordering of the view
    std::map<data_key, sort_order> _sort_order = nao_view::list_view_default_sort();
    data_key _selected_column = KEY_NAME;

    public:
    list_view_preview(nao_view& view, item_file_handler* handler);

    ~list_view_preview() override = default;

    list_view* list() const;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

class push_button;
class slider;
class label;
class line_edit;
class separator;
class seekable_progress_bar;

// A preview which plays audio
class audio_player_preview : public preview {
    icon _play_icon;
    icon _pause_icon;

    std::unique_ptr<seekable_progress_bar> _progress_bar;
    std::unique_ptr<push_button> _toggle_button;
    std::unique_ptr<slider> _volume_slider;
    std::unique_ptr<label> _volume_display;
    std::unique_ptr<label> _progress_display;
    std::unique_ptr<label> _duration_display;

    std::unique_ptr<separator> _separator;

    std::unique_ptr<label> _codec_label;
    std::unique_ptr<line_edit> _codec_edit;

    std::unique_ptr<label> _rate_label;
    std::unique_ptr<line_edit> _rate_edit;

    std::unique_ptr<label> _channels_label;
    std::unique_ptr<line_edit> _channels_edit;

    std::unique_ptr<label> _order_label;
    std::unique_ptr<line_edit> _order_edit;

    std::unique_ptr<label> _type_label;
    std::unique_ptr<line_edit> _type_edit;

    std::unique_ptr<audio_player> _player;

    std::chrono::nanoseconds _duration {};

    dimensions _volume_display_size {};

    dimensions _progress_size {};
    dimensions _duration_size {};

    bool _resume_after_seek = false;

    public:
    explicit audio_player_preview(nao_view& view, std::unique_ptr<audio_player> player);

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void _set_progress(std::chrono::nanoseconds progress);
};

class direct2d_image_display;

// Display a single image
class image_viewer_preview : public preview {
    std::unique_ptr<direct2d_image_display> _window;

    public:
    explicit image_viewer_preview(nao_view& view, std::unique_ptr<image_provider> image);

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};



// Plays video (and optionally audio)
class video_player_preview : public preview, IMFAsyncCallback {
    enum state {
        closed,
        open_pending,
        ready,
        closing,
        started,
        paused,
        stopped
    } _state = closed;

    av_file_handler* _handler;

    volatile uint32_t _refcount = 1;

    com_ptr<IMFMediaSource> _source;
    //com_ptr<IMFMediaSession> _session;
    mf::media_session _session;
    com_ptr<IMFVideoDisplayControl> _display;
    std::unique_ptr<mf::binary_stream_imfbytestream> _bs;
    HANDLE _close_event;

    public:
    video_player_preview(nao_view& view, av_file_handler* handler);
    ~video_player_preview();

    void pause();
    void stop();
    void repaint();
    void resize(LONG width, LONG height);
    void handle_event(IMFMediaEvent* ev);

    void session_topology_status(const com_ptr<IMFMediaEvent>& event);
    void presentation_ended(const com_ptr<IMFMediaEvent>& event);
    void new_presentation(const com_ptr<IMFMediaEvent>& event);

    void start_playback();
    void play();

    static constexpr UINT WM_APP_PLAYER_EVENT = WM_APP + 1;

    protected:
    bool wm_create(CREATESTRUCTW*) override;
    void wm_paint() override;
    void wm_size(int type, int width, int height) override;

    private:
    STDMETHODIMP QueryInterface(const IID& riid, void** ppvObject) override { static const QITAB qit[] { QITABENT(video_player_preview, IMFAsyncCallback), { } }; return QISearch(this, qit, riid, ppvObject); }
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&_refcount); }
    STDMETHODIMP_(ULONG) Release() override { uint32_t count = InterlockedDecrement(&_refcount); if (count == 0) { delete this; } return count; }
    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) override { return E_NOTIMPL; }
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) override;


    bool _create_session();
    bool _create_source(const istream_ptr& stream);
    bool _create_topology(IMFPresentationDescriptor* pd, HWND hwnd, IMFTopology** topology);
    bool _close_session();

    void _add_branch(IMFTopology* topology, uint32_t index, HWND hwnd,
        IMFPresentationDescriptor* pd);

    void _create_sink_activate(IMFStreamDescriptor* sd, HWND hwnd, IMFActivate** activate);

    void _add_source_node(IMFTopology* topology, IMFPresentationDescriptor* pd, IMFStreamDescriptor* sd, IMFTopologyNode** node);
    void _add_output_node(IMFTopology* topology, IMFActivate* activate, DWORD id, IMFTopologyNode** node);

    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
