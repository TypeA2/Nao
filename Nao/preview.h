#pragma once

#include "file_handler.h"
#include "nao_view.h"

#include "audio_player.h"

#include <chrono>

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

template <typename T>
class preview_t : public preview {
    public:
    preview_t(nao_view& view, T* handler)
        : preview(view), handler(handler) { }
    virtual ~preview_t() = default;

    protected:
    T* handler;
};

using preview_ptr = std::unique_ptr<preview>;

class list_view;

// Multi-use list-view preview
class list_view_preview : public preview_t<item_file_handler> {
    public:
    list_view_preview(nao_view& view, item_file_handler* handler);

    ~list_view_preview() override = default;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    std::unique_ptr<list_view> _m_list;

    // Current column ordering of the view
    std::map<data_key, sort_order> _m_sort_order;
    data_key _m_selected_column;
};

class push_button;
class slider;
class label;
class seekable_progress_bar;

// A preview which plays audio
class audio_player_preview : public preview_t<pcm_file_handler> {
    public:
    explicit audio_player_preview(nao_view& view, pcm_file_handler* handler);

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void _set_progress(std::chrono::nanoseconds progress);

    icon _m_play_icon;
    icon _m_pause_icon;

    std::unique_ptr<seekable_progress_bar> _m_progress_bar;
    std::unique_ptr<push_button> _m_toggle_button;
    std::unique_ptr<slider> _m_volume_slider;
    std::unique_ptr<label> _m_volume_display;
    std::unique_ptr<label> _m_progress_display;
    std::unique_ptr<label> _m_duration_display;

    std::unique_ptr<audio_player> _m_player;

    std::chrono::nanoseconds _m_duration;

    size _m_volume_display_size;

    size _m_progress_size;
    size _m_duration_size;

    bool _m_resume_after_seek;
};
