#pragma once

#include "ui_element.h"

class nao_view;

class right_window : public ui_element {
    public:
    explicit right_window(ui_element* parent, nao_view* view);
    ~right_window() override = default;

    //void set_preview(const std::shared_ptr<ui_element>& element, preview_type type);
    //void clear_preview();

    protected:
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    //LRESULT _wnd_proc_list_view(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
