#pragma once

#include "ui_element.h"

enum progress_bar_message : UINT {
    // WPARAM is the current value
    PB_CAPTURE = WM_APP + 1,
    PB_SEEK,
    PB_RELEASE
};

class seekable_progress_bar : public ui_element {
    public:
    static constexpr int64_t handle_size = 3;

    seekable_progress_bar(ui_element* parent, uintmax_t from, uintmax_t to);
    ~seekable_progress_bar() override;

    void set_progress(uintmax_t value);

    protected:
    void wm_paint() override;

    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
    void _draw_fill(HDC hdc, bool clear_nondrawn = false) const;

    HPEN _m_background_pen;
    HBRUSH _m_background_brush;

    HPEN _m_foreground_pen;
    HBRUSH _m_foreground_brush;

    uintmax_t _m_min;
    uintmax_t _m_max;
    uintmax_t _m_current;

    HCURSOR _m_hand_cursor;
    HCURSOR _m_old_cursor;

    bool _m_is_dragging;
};