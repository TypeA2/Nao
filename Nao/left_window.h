#pragma once

#include "list_view.h"
#include "push_button.h"
#include "line_edit.h"

class nao_view;

class left_window : public ui_element {
    nao_view& _view;

    list_view _list;
    push_button _up;
    push_button _refresh;
    push_button _browse;
    line_edit _path;

    public:
    explicit left_window(ui_element* parent, nao_view& view);
    ~left_window() override = default;

    line_edit& path();
    push_button& view_up();
    list_view& list();

    protected:
    void wm_size(int, const dimensions& dims) override;
    void wm_command(WORD, WORD, HWND target) override;
    void wm_notify(WPARAM, NMHDR* nm) override;

};
