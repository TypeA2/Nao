#pragma once

#include "ui_element.h"

enum progress_bar_message : UINT {
    // WPARAM is the current value
    PB_CAPTURE = WM_APP + 1,
    PB_SEEK,
    PB_RELEASE
};

class seekable_progress_bar : public ui_element {
    HCURSOR _hand = LoadCursorW(nullptr, IDC_HAND);
    HCURSOR _norm = LoadCursorW(nullptr, IDC_ARROW);

    win32::pen _bg_pen { CreatePen(PS_SOLID, 1, RGB(0, 0, 0)) };
    win32::brush _bg_brush { CreateSolidBrush(RGB(0xff, 0xff, 0xff)) };

    win32::pen _fg_pen { CreatePen(PS_NULL, 0, 0) };
    win32::brush _fg_brush { CreateSolidBrush(RGB(0x4d, 0xb2, 0xff)) };

    uintmax_t _min;
    uintmax_t _max;
    uintmax_t _current = 0;

    bool _is_dragging = false;

    public:
    static constexpr int64_t handle_size = 3;

    seekable_progress_bar(ui_element* parent, uintmax_t from, uintmax_t to);
    
    void set_progress(uintmax_t value);

    protected:
    void wm_paint() override;

    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
    void _draw_fill(const win32::device_context& dc, bool clear_nondrawn = false) const;
};