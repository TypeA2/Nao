#pragma once

#include "ui_element.h"

class list_view;
class line_edit;
class push_button;
class nao_view;

class left_window : public ui_element {
    std::unique_ptr<list_view> _m_list;
    std::unique_ptr<push_button> _m_up;
    std::unique_ptr<push_button> _m_refresh;
    std::unique_ptr<push_button> _m_browse;
    std::unique_ptr<line_edit> _m_path;

    public:
    explicit left_window(ui_element* parent, nao_view* view);
    ~left_window() override = default;

    line_edit* path() const;
    push_button* view_up() const;
    list_view* list() const;

    protected:
    bool wm_create(CREATESTRUCTW* create) override;
    void wm_size(int type, int width, int height) override;
    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

    nao_view* view;
};
