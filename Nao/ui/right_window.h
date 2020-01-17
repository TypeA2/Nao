#pragma once

#include "ui_element.h"

class data_model;

class right_window : public ui_element {
    public:
    right_window(ui_element* parent, data_model& model);
    ~right_window();

    void set_preview(ui_element* element);
    void clear_preview();

    ui_element* preview() const;

    protected:
    void wm_size(int type, int width, int height) override;

    private:
    void _init();

    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    data_model& _m_model;

    ui_element* _m_preview;
};
