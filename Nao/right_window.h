#pragma once

#include "ui_element.h"
#include "nao_view.h"
#include "preview.h"

class right_window : public ui_element {
    public:
    explicit right_window(ui_element* parent, nao_view* view);
    ~right_window() override = default;

    void set_preview(preview_ptr instance);
    void remove_preview();
    preview* get_preview() const;

    protected:
    void wm_size(int type, int width, int height) override;

    private:
    LRESULT _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    preview_ptr _m_preview;
};
