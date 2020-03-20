#include "seekable_progress_bar.h"

#include "utils.h"

#include "resource.h"

#include <algorithm>

seekable_progress_bar::seekable_progress_bar(ui_element* parent, uintmax_t from, uintmax_t to)
    : ui_element(parent, win32::wnd_class {
            .class_name = win32::load_wstring(IDS_PROGRESS_BAR),
            .background = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1) }, WS_CHILD | WS_VISIBLE)
    , _min { from }, _max { to } {

    ASSERT(from < to);
}

void seekable_progress_bar::set_progress(uintmax_t value) {
    ASSERT(value >= _min && value <= _max);

    _current = value;

    ASSERT(redraw(RDW_INVALIDATE));
}


void seekable_progress_bar::wm_paint() {
    win32::paint_struct ps { this };

    ps.select(_bg_pen);
    ps.select(_bg_brush);
    ps.rectangle(dims().rect());

    _draw_fill(ps);
}


LRESULT seekable_progress_bar::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_MOUSEMOVE: {
            coordinates at {
                .x = GET_X_LPARAM(lparam),
                .y = GET_Y_LPARAM(lparam)
            };
            
            double frac = _current / static_cast<double>(_max);
            int64_t handle_pos = static_cast<int64_t>((width() - 1.) * frac);

            if (handle_pos >= (at.x - handle_size) && handle_pos <= (at.x + handle_size)) {
                SetCursor(_hand);
            } else {
                SetCursor(_norm);
            }

            if (_is_dragging) {
                double pixel_frac = (at.x - 1.) / (width() - 1.);
                double target = round(_max * pixel_frac);
                if (target < _min) {
                    target = static_cast<double>(_min);
                } else if (target > _max) {
                    target = static_cast<double>(_max);
                }

                _current = static_cast<uintmax_t>(target);

                _draw_fill(dc(), true);

                (void) parent()->send_message(PB_SEEK, _current);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            coordinates at {
                .x = GET_X_LPARAM(lparam),
                .y = GET_Y_LPARAM(lparam)
            };

            double percent = _current / static_cast<double>(_max);
            int64_t handle_pos = static_cast<int64_t>((width() - 1i64) * percent);

            if (handle_pos >= (at.x - handle_size) && handle_pos <= (at.x + handle_size)) {
                _is_dragging = true;
                SetCapture(handle());

                (void) parent()->send_message(PB_CAPTURE, _current);
            }

            break;
        }

        case WM_LBUTTONUP: {
            if (_is_dragging) {
                _is_dragging = false;
                ReleaseCapture();

                (void) parent()->send_message(PB_RELEASE, _current);
            }
        }

        default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
}

void seekable_progress_bar::_draw_fill(const win32::device_context& dc, bool clear_nondrawn) const {
    auto old_pen = dc.temporary_select(_fg_pen);
    auto old_brush = dc.temporary_select(_fg_brush);

    auto [width, height] = dims();

    double percent = _current / static_cast<double>(_max);
    int drawn_width = utils::narrow<int>(round(width * percent));

    dc.rectangle({ 1, 1, drawn_width, height });

    if (clear_nondrawn) {
        dc.select(win32::stock_object(NULL_PEN));
        dc.select(_bg_brush);
        dc.rectangle({ std::max(drawn_width, 1), 1, width, height });
    }
}

