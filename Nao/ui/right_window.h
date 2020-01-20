#pragma once

#include "ui_element.h"
#include "data_model.h"

class right_window : public ui_element {
    public:
    using preview_type = data_model::preview_type;

    right_window(ui_element* parent, data_model& model);
    ~right_window();

    void set_preview(ui_element* element, preview_type type);
    void clear_preview();

    ui_element* preview() const;
    preview_type type() const;

    protected:
    void wm_size(int type, int width, int height) override;

    private:
    void _init();

    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT _wnd_proc_list_view(UINT msg, WPARAM wparam, LPARAM lparam);

    data_model& _m_model;

    ui_element* _m_preview;
    preview_type _m_type;
};
