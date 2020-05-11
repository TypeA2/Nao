#pragma once

#include "file_handler.h"
#include "nao_view.h"

#include "audio_player.h"

#include "list_view.h"
#include "seekable_progress_bar.h"
#include "push_button.h"
#include "slider.h"
#include "label.h"
#include "separator.h"
#include "line_edit.h"
#include "sdl_image_display.h"

#include <chrono>

#include "mf.h"

class nao_controller;

// Wrapper class for preview elements

class preview : public ui_element {
    protected:
    nao_view& view;
    nao_controller& controller;

    public:
    explicit preview(nao_view& view);
    preview(nao_view& view, int string);

    virtual ~preview() = default;
};

using preview_ptr = std::unique_ptr<preview>;

// Multi-use list-view preview
class list_view_preview : public preview {
    item_file_handler* _handler;

    list_view _list;

    // Current column ordering of the view
    std::map<data_key, sort_order> _sort_order = nao_view::list_view_default_sort();
    data_key _selected_column = KEY_NAME;

    public:
    list_view_preview(nao_view& view, item_file_handler* handler);

    ~list_view_preview() override = default;

    list_view& list();

    protected:
    void wm_size(int, const dimensions& dims) override;
    void wm_notify(WPARAM, NMHDR* hdr) override;
};

// A preview which plays audio
class audio_player_preview : public preview {
    std::unique_ptr<audio_player> _player;

    seekable_progress_bar _progress_bar;
    push_button _toggle_button;
    slider _volume_slider;
    label _volume_display;
    label _progress_display;
    label _duration_display;

    separator _separator;

    struct info_pair {
        info_pair(ui_element* parent,
            const std::string& label, const std::string& edit);

        void move(defer_window_pos& dwp,
            int64_t partial_offset, int64_t partial_width,
            int64_t info_offset, int64_t index);

        label label;
        line_edit edit;
    };

    info_pair _codec;
    info_pair _rate;
    info_pair _channels;
    info_pair _type;

    win32::icon _play_icon;
    win32::icon _pause_icon;

    std::chrono::nanoseconds _duration {};

    dimensions _volume_display_size {};

    dimensions _progress_size {};
    dimensions _duration_size {};

    bool _resume_after_seek = false;

    public:
    explicit audio_player_preview(nao_view& view, std::unique_ptr<audio_player> player);

    protected:
    void wm_size(int, const dimensions& dims) override;
    void wm_command(WORD id, WORD code, HWND target) override;

    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
    void _set_progress(std::chrono::nanoseconds progress);
};

// Display a single image
class image_viewer_preview : public preview {
    sdl_image_display _window;

    public:
    explicit image_viewer_preview(nao_view& view, const image_data& data);

    protected:
    void wm_size(int, const dimensions& dims) override;
};



// Plays video (and optionally audio)
class video_player_preview : public preview {
    static constexpr int64_t display_margin = 200;

    class player_canvas : public ui_element {
        static rectangle start_rect(ui_element* parent);

        public:
        explicit player_canvas(ui_element* parent);
    }  _player_display;
    mf::player _player;

    win32::icon _play_icon;
    win32::icon _pause_icon;

    push_button _toggle_button;

    public:
    video_player_preview(nao_view& view, av_file_handler* handler);
    ~video_player_preview() = default;

    protected:
    void wm_paint() override;
    void wm_size(int, const dimensions& dims) override;

    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;
};
