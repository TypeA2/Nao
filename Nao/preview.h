#pragma once

#include "ui_element.h"
#include "nao_view.h"

class nao_controller;

// Wrapper class for preview elements
class preview : public ui_element {
    public:
    explicit preview(nao_view& view, item_provider* provider);
    virtual ~preview();

    protected:

    static std::wstring register_once(int id);

    nao_view& view;
    nao_controller& controller;
    item_provider* provider;
};

using preview_ptr = std::unique_ptr<preview>;

class list_view;

// Multi-use list-view preview
class list_view_preview : public preview {
    public:
    explicit list_view_preview(nao_view& view, item_provider* provider);
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
class audio_player;

// A preview which plays audio
class audio_player_preview : public preview {
    public:
    explicit audio_player_preview(nao_view& view, item_provider* provider);

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    icon _m_play_icon;
    icon _m_pause_icon;

    std::unique_ptr<push_button> _m_toggle_button;
    std::unique_ptr<audio_player> _m_player;
};
